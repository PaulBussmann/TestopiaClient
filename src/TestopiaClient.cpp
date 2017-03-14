/**************************************************************************
 *
 * The GNU Lesser General Public License (LGPL)
 * 
 * Copyright (c) 2015 PaulBussmann
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

#include "TestopiaClient.h"
#include "TestopiaRpcClient.h"

static TestopiaRpcClient* rpcClient = NULL;

#if TESTOPIA_CLIENT_THREADING
#include <thread>	
#include <mutex>
std::mutex testCaseRunInitQueueMutex;

const char* TestopiaClientCppUTestPlugin::GetVersion()
{
    return TESTOPIACLIENT_VERSION;
}

void TestopiaClientCppUTestPlugin::QueueTestCaseRunInit(const char* name)
{
#ifdef DEBUG_PRINTF
	printf("QueueTestCaseRunInit(%s)\n", name);
#endif
	std::string strName = name;
	testCaseRunInitQueueMutex.lock();
	testCaseRunInitQueue.push(strName);
	testCaseRunInitQueueMutex.unlock();
}
#endif

void TestopiaClientCppUTestPlugin::TestCaseRunInit(const char* name)
{
	if (!rpcClient)
		return;
#ifdef DEBUG_PRINTF
	printf("TestopiaClientCppUTestPlugin::TestCaseRunInit(%s)\n", name);
#endif

	int testCaseId = rpcClient->TestCaseGetIdByPlanIdAndSummary(planId, name);

	if (0 == testCaseId)
		testCaseId = rpcClient->TestCaseGetIdByPlanIdAndSummary(0, name);

	if (0 != testCaseId)
		rpcClient->TestRunAddCase(testCaseId, runId);

	rpcClient->TestCaseRunUpdate(name,
			(int)TestCaseStatus_IDLE, planId, runId, buildId, envId, testCaseId);
}
void TestopiaClientCppUTestPlugin::SetTestCaseIdleTestOutput::print(const char* name) {
	if (!rpcClient)
		return;

	SimpleString groupAndNameList(name);
	SimpleStringCollection groupAndNameArray;
	size_t i;

	groupAndNameList.split(" ", groupAndNameArray);

	for (i = 0; i < groupAndNameArray.size(); i++) {
		SimpleString groupAndName(groupAndNameArray[i].asCharString());
		groupAndName.replace(".", ", ");

		if (groupAndName.endsWith(" "))
			groupAndName = groupAndName.subString(0, groupAndName.size() - 1);

#if TESTOPIA_CLIENT_THREADING
		testopiaClientPlugin->QueueTestCaseRunInit(groupAndName.asCharString());
#else
		testopiaClientPlugin->TestCaseRunInit(groupAndName.asCharString());
#endif
	}
}

TestopiaClientCppUTestPlugin::TestopiaClientCppUTestPlugin(const char* user, const char* pass,
		const char* host, unsigned port, unsigned to_sec) :
		TestPlugin("SyncTestopia") {
	failureAction = FA_TEARDOWN_TEST;
			
	product = "";
	productId = 0;
    productVersion = "";
	buildId = 0;
    build = "";
    buildDescription = "";
	envId = 0;
	environment = "";
	planId = 0;
	runId = 0;
    runSummary = "";
    runManagerName = "";
    runManagerId = 0;

#if TESTOPIA_CLIENT_THREADING
    rpcClientWorkerStop = false;
#endif

	ulxr::CppString strUser = ULXR_PCHAR(user);
	ulxr::CppString strPassword = ULXR_PCHAR(pass);
	ulxr::CppString strHost = ULXR_PCHAR(host);
	rpcClient = new TestopiaRpcClient(strUser, strPassword, strHost, port,
			to_sec);
#if TESTOPIA_CLIENT_THREADING
    rpcClientWorkerError = false;
#endif
}

TestopiaClientCppUTestPlugin::~TestopiaClientCppUTestPlugin() {
	delete rpcClient;
}

#if TESTOPIA_CLIENT_THREADING
void TestopiaClientCppUTestPlugin::rpcClientWorker(void)
{
#ifdef TESTTOPIA_CLIENT_BACKGROUND
	if (!SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN))
	{
		DWORD dwError = GetLastError();
		if (ERROR_THREAD_MODE_ALREADY_BACKGROUND == dwError)
			printf("Already in background mode\n");
		else printf("Failed to enter background mode (%d)\n", dwError);
	}
#endif

	rpcClientWorkerError = false;

	try
	{
		TestRunCreate();

		while (!rpcClientWorkerStop)
		{
			testCaseRunInitQueueMutex.lock();
			while (!testCaseRunInitQueue.empty())
			{
				std::string testCaseName = testCaseRunInitQueue.front();
				testCaseRunInitQueue.pop();
				testCaseRunInitQueueMutex.unlock();

				TestCaseRunInit(testCaseName.c_str());

				testCaseRunInitQueueMutex.lock();
			}
			while (!testCaseStatusChangeQueue.empty())
			{
				TestCaseNameAndStatus caseStatusChange = testCaseStatusChangeQueue.front();
				testCaseStatusChangeQueue.pop();
				testCaseRunInitQueueMutex.unlock();

				TestCaseRunUpdate(caseStatusChange.name.c_str(), caseStatusChange.status);

				testCaseRunInitQueueMutex.lock();
			}
			testCaseRunInitQueueMutex.unlock();
			sleep(10);
		}
	}
	catch (ConnectionException& e)
	{
		std::cout << e.what() << " " << e.why();
		rpcClientWorkerError = true;
	}
}
void staticRpcClientWorker(TestopiaClientCppUTestPlugin* testopiaClient)
{
	testopiaClient->rpcClientWorker();
}
#endif

void TestopiaClientCppUTestPlugin::TestRunCreate()
{
	productId = rpcClient->ProductGetIdByName(product);

	// root@testopia:/var/www/html/Bugzilla/Version.pm
	// How to create version via HTTP?

	buildId = rpcClient->BuildGetIdByProductNameAndBuildName(product, build);
	if (0 == buildId)
		rpcClient->BuildCreate(build, productId, 0, buildDescription);

	runId = rpcClient->TestRunCreate(planId, environment, build, runSummary, runManagerName, productVersion, TestRunStatus_RUNNING,
		envId, buildId, runManagerId);
}

void TestopiaClientCppUTestPlugin::TestRunStop()
{
	rpcClient->TestRunStatusUpdate(runId, TestRunStatus_STOPPED);
}

int TestopiaClientCppUTestPlugin::runRunAllTests(int ac, char** av) {
	if (!rpcClient)
		return -1;

	int failCount = 0;

#if TESTOPIA_CLIENT_THREADING
    rpcClientWorkerStop = false;

	std::thread worker(
#if (defined LPTHREAD_START_ROUTINE) || (defined WIN32)
		(LPTHREAD_START_ROUTINE)
#endif
		staticRpcClientWorker, this);
#else
	TestRunCreate();
#endif

	CommandLineArguments arguments(ac, (const char**) av);
	arguments.parse(this);

	TestRegistry::getCurrentRegistry()->setGroupFilters(
			arguments.getGroupFilters());
	TestRegistry::getCurrentRegistry()->setNameFilters(
			arguments.getNameFilters());
	SetTestCaseIdleTestOutput testOutput(this, runId);
	TestResult testResult(testOutput);

	TestRegistry::getCurrentRegistry()->listTestGroupAndCaseNames(testResult);
	failCount = CommandLineTestRunner::RunAllTests(ac, av);

#if TESTOPIA_CLIENT_THREADING
    rpcClientWorkerStop = true;
#ifdef DEBUG_PRINTF
    printf("worker.join()\n");
#endif
	worker.join();
#endif
	try
	{
		TestRunStop();
	}
	catch (ConnectionException& e)
	{
		std::cout << e.what() << " " << e.why();
#if TESTOPIA_CLIENT_THREADING
        rpcClientWorkerError = true;
#endif
	}

	return failCount;
}

// Add to testCaseStatusChangeQueue
#if TESTOPIA_CLIENT_THREADING
void TestopiaClientCppUTestPlugin::QueueTestCaseRunUpdate(const char* name, TestCaseStatus status)
{
#ifdef DEBUG_PRINTF
	printf("QueueTestCaseRunUpdate(%s)\n", name);
#endif
	struct TestCaseNameAndStatus caseStatusChange;
	caseStatusChange.name = name;
	caseStatusChange.status = status;
	testCaseRunInitQueueMutex.lock();
	testCaseStatusChangeQueue.push(caseStatusChange);
	testCaseRunInitQueueMutex.unlock();
}
#endif

void TestopiaClientCppUTestPlugin::TestCaseRunUpdate(const char* name, TestCaseStatus status) {
	if (!rpcClient)
		return;

	rpcClient->TestCaseRunUpdate(name, (int) status, planId, runId,
			buildId, envId, 0);
}

