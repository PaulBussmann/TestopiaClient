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

#ifndef TESTOPIACLIENT_H_
#define TESTOPIACLIENT_H_

#define TESTOPIACLIENT_VERSION "0.9"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestRegistry.h"

#include "CppUTest/TestOutput.h"
#include "CppUTest/TestPlugin.h"

#include <assert.h>
#define TESTOPIA_CLIENT_THREADING 1
#ifdef __linux__
#else
#endif
#if TESTOPIA_CLIENT_THREADING
#include <queue>
#if 1
#endif
#endif

#if (!defined sleep) && (!defined __linux__)
#define sleep _sleep
#endif

/// Creates Test Run, Test Cases and updates Test Case Runs in Testopia
class TestopiaClientCppUTestPlugin : public TestPlugin
{
public:
	/// returns "<GroupName>, <TestName>". Same as command line test runner does in verbose mode.
	static inline const char* TestGroupAndNameString(UtestShell& test, SimpleString& groupAndName)
	{
		groupAndName = test.getGroup();
		groupAndName += ", ";
		groupAndName += test.getName();
		return groupAndName.asCharString();
	}

	/// Assign Testopia test case state names to number
	typedef enum
	{
		TestCaseStatus_IDLE = 1,
		TestCaseStatus_PASSED = 2,
		TestCaseStatus_FAILED = 3,
		TestCaseStatus_RUNNING = 4,
	} TestCaseStatus;

	/// Assign Testopia test run state names to number
	typedef enum
	{
		TestRunStatus_STOPPED = 0, // TODO
		TestRunStatus_RUNNING = 1,
	} TestRunStatus;

#if TESTOPIA_CLIENT_THREADING
	struct TestCaseNameAndStatus
	{
	public:
		std::string name;
		TestCaseStatus status;
	};
	/// Creates test run and updates test cases on request in Testopia (multi-threading)
	void rpcClientWorker(void);
	/// Request bit read by Client Worker
	bool rpcClientWorkerStop;
	bool rpcClientWorkerError;
#endif

	/// TODO This is a runner API, not a PlugIn-API!
    int runRunAllTests(int ac, char** av);

	class CommandLineTestRunner : public ::CommandLineTestRunner
	{
	public:
		
		CommandLineTestRunner(int ac, const char** av, TestRegistry* registry,
			TestopiaClientCppUTestPlugin* client)
			: ::CommandLineTestRunner(ac, av, registry)
		{this->client = client;}
	private:
		TestopiaClientCppUTestPlugin* client;
	};

	/// Dirty hack to get list of Tests to be run
	class SetTestCaseIdleTestOutput : public ::TestOutput
	{
	public:
		SetTestCaseIdleTestOutput(TestopiaClientCppUTestPlugin* client, int runIdParam) {
			testopiaClientPlugin = client;
			runId = runIdParam;
		}
		/// Interfacing CppUTest->TestopiaClientCppUTestPlugin: called once per Test that will be part of Test Run
		virtual void print(const char* name);
		virtual void printBuffer(const char*) {}
	    virtual void flush() {}
	    int runId;
	private:
	    TestopiaClientCppUTestPlugin* testopiaClientPlugin;
	};
	
	TestopiaClientCppUTestPlugin(const char* user, const char* pass,
			const char* host = "localhost", unsigned port = 80, unsigned to_sec = 30);
	virtual ~TestopiaClientCppUTestPlugin();

	/// Creates Test _Run_
	void TestRunCreate();

	/// Stops Test _Run_
    void TestRunStop();

	/// Creates Test _Case_ if non-existing and sets it to IDLE
	void TestCaseRunInit(const char* name);
#if TESTOPIA_CLIENT_THREADING
	void QueueTestCaseRunInit(const char* name);
#endif

#if TESTOPIA_CLIENT_THREADING
	/// Add to testCaseStatusChangeQueue
	void QueueTestCaseRunUpdate(const char* name, TestCaseStatus status);
#endif
	/// Update test case run at Testopia
	void TestCaseRunUpdate(const char* name, TestCaseStatus status);


	/// Interfacing CppUTest -> Testopia: Set TestCaseRun state "RUNNING" at Testopia when CppUTest Test _Case_ started
	virtual void preTestAction(UtestShell& test, TestResult& result)
    {
    	SimpleString groupAndName;
#if TESTOPIA_CLIENT_THREADING
		QueueTestCaseRunUpdate(TestGroupAndNameString(test, groupAndName), TestopiaClientCppUTestPlugin::TestCaseStatus_RUNNING);
#else
		TestCaseRunUpdate(TestopiaClientCppUTestPlugin::TestCaseStatus_RUNNING);
#endif
    }

	/// Interfacing CppUTest -> Testopia: Set TestCaseRun state "FAILED"/"PASSED" at Testopia when CppUTest Test _Case_ finished
	virtual void postTestAction(UtestShell& test, TestResult& result)
    {
    	SimpleString groupAndName;
    	TestopiaClientCppUTestPlugin::TestCaseStatus status;

    	if (test.hasFailed())
    		status = TestopiaClientCppUTestPlugin::TestCaseStatus_FAILED;
    	else
    		status = TestopiaClientCppUTestPlugin::TestCaseStatus_PASSED;

#if TESTOPIA_CLIENT_THREADING
    	QueueTestCaseRunUpdate(TestGroupAndNameString(test, groupAndName), status);
#else
		TestCaseRunUpdate(TestGroupAndNameString(test, groupAndName), status);
#endif
		
		// TODO: if exit if any test fails, abort all other tests!
    	if (test.hasFailed())
		{
			switch (failureAction)
			{
			case FA_CONTINUE_TEST:
			case FA_TEARDOWN_TEST:
				break;
			case FA_TERMINATE_APPLICATION:
				// TODO
			default:
			case FA_EXIT_APPLICATION:
				assert(false);
				break;
			}
		}
    }

	typedef enum _FAILURE_ACTION 
	{
		FA_CONTINUE_TEST,
		FA_TEARDOWN_TEST, // Default
		FA_TERMINATE_APPLICATION,
		FA_EXIT_APPLICATION
	} FAILURE_ACTION;
	FAILURE_ACTION failureAction;

	const char* product;
    int productId;
    const char* productVersion;
    int buildId;
    const char* build;
    const char* buildDescription;
    int envId;
    const char* environment;
    int planId;
    int runId;
    const char* runSummary;
    const char* runManagerName;
    int runManagerId;

#if TESTOPIA_CLIENT_THREADING
    std::queue<std::string> testCaseRunInitQueue;
    std::queue<struct TestCaseNameAndStatus> testCaseStatusChangeQueue;
#endif
};

#endif /* TESTOPIACLIENT_H_ */
