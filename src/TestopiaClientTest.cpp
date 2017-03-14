#include "TestopiaRpcClient.h"
#include "TestopiaClient.h"

#ifdef TESTOPIA_CLIENT_TEST

#define TESTOPIA_SERVER_HOSTNAME "localhost"
#define TESTOPIA_USERNAME "admin@localhost.localdomain"
#define TESTOPIA_PASSWORD "password"
static const int C_testopiaClientProductId = 1;

TEST_GROUP(TestopiaRpcClient)
{
};

TEST(TestopiaRpcClient, TestTestCaseGetIdByPlanIdAndSummary)
{
	ulxr::CppString strUser = ULXR_PCHAR(TESTOPIA_USERNAME);
	ulxr::CppString strPassword = ULXR_PCHAR(TESTOPIA_PASSWORD);
	ulxr::CppString strHost = ULXR_PCHAR(TESTOPIA_SERVER_HOSTNAME);
	TestopiaRpcClient rpcClient(strUser, strPassword, strHost, 80, 90);

	int testCaseId = 0;
	try {
		testCaseId = rpcClient.TestCaseGetIdByPlanIdAndSummary(1, "TestGroup, Test");
	}
	catch(...) { FAIL("Unhandled exception!"); }
	CHECK_EQUAL(1, testCaseId);
}


TEST(TestopiaRpcClient, TestEnvironmentGetIdByName)
{
	ulxr::CppString strUser = ULXR_PCHAR(TESTOPIA_USERNAME);
	ulxr::CppString strPassword = ULXR_PCHAR(TESTOPIA_PASSWORD);
	ulxr::CppString strHost = ULXR_PCHAR(TESTOPIA_SERVER_HOSTNAME);
	TestopiaRpcClient rpcClient(strUser, strPassword, strHost, 80, 90);

	int envId = 0;
	try {
		envId = rpcClient.EnvironmentGetIdByName("Windows");
	}
	catch(...) { FAIL("Unhandled exception!"); }
	CHECK_EQUAL(1, envId);
}

#if 0
// disabled, because build cannot be deleted with XMLRPC
TEST(TestopiaRpcClient, TestBuildCreate)
{
	ulxr::CppString strUser = ULXR_PCHAR(TESTOPIA_USERNAME);
	ulxr::CppString strPassword = ULXR_PCHAR(TESTOPIA_PASSWORD);
	ulxr::CppString strHost = ULXR_PCHAR(TESTOPIA_SERVER_HOSTNAME);
	TestopiaRpcClient rpcClient(strUser, strPassword, strHost, 80, 90);

	int buildId;

	try {
		buildId = rpcClient.BuildCreate("V0.0.1.03", 1, 0 /* isActive TODO: semantic? */, "TEST(TestopiaRpcClient, TestBuildCreate)");
	}
	catch(...) { FAIL("Unhandled exception!"); }
	CHECK_EQUAL(3, buildId);
}
#endif

TEST(TestopiaRpcClient, TestGetBuildIdByProductNameAndBuildName)
{
	ulxr::CppString strUser = ULXR_PCHAR(TESTOPIA_USERNAME);
	ulxr::CppString strPassword = ULXR_PCHAR(TESTOPIA_PASSWORD);
	ulxr::CppString strHost = ULXR_PCHAR(TESTOPIA_SERVER_HOSTNAME);
	TestopiaRpcClient rpcClient(strUser, strPassword, strHost, 80, 90);

	int buildId;

	try {
		buildId = rpcClient.BuildGetIdByProductNameAndBuildName("TestopiaClient", "V0.9");
	}
	catch(...) { FAIL("Unhandled exception!"); }
	CHECK_EQUAL(1, buildId);
}

TEST(TestopiaRpcClient, TestProductGetByName)
{
	ulxr::CppString strUser = ULXR_PCHAR(TESTOPIA_USERNAME);
	ulxr::CppString strPassword = ULXR_PCHAR(TESTOPIA_PASSWORD);
	ulxr::CppString strHost = ULXR_PCHAR(TESTOPIA_SERVER_HOSTNAME);
	TestopiaRpcClient rpcClient(strUser, strPassword, strHost, 80, 90);

	int productId;

	try {
		productId = rpcClient.ProductGetIdByName("TestopiaClient");
	}
    catch (...) { FAIL("Unhandled exception!"); }
	CHECK_EQUAL(C_testopiaClientProductId, productId);
}

#if 0
TEST(TestopiaRpcClient, CurrentTest)
{
	ulxr::CppString strUser = ULXR_PCHAR(TESTOPIA_USERNAME);
	ulxr::CppString strPassword = ULXR_PCHAR(TESTOPIA_PASSWORD);
	ulxr::CppString strHost = ULXR_PCHAR(TESTOPIA_SERVER_HOSTNAME);
	TestopiaRpcClient rpcClient(strUser, strPassword, strHost, 80, 90);

	try {
		rpcClient.TestRunStatusUpdate(19, 0 /* TestRunStatus_STOPPED */);
	}
	catch (ulxr::Exception &ex) {
		ULXR_COUT<< ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why())
			<< std::endl;
	}
	catch (...) {
		ULXR_COUT << ULXR_PCHAR("unknown Error occured.\n");
	}
}
#endif

int main(int nArgc, char* ppArgv[])
{
#if 0
	return CommandLineTestRunner::RunAllTests(nArgc, ppArgv);
#else
	MemoryLeakWarningPlugin::getFirstPlugin()->turnOffNewDeleteOverloads();


	TestopiaClientCppUTestPlugin testopiaClient(TESTOPIA_USERNAME, TESTOPIA_PASSWORD, TESTOPIA_SERVER_HOSTNAME, 80, 900);

	testopiaClient.buildDescription = "Added by ConnectionStateMachineTest";
	testopiaClient.runManagerName = TESTOPIA_USERNAME;

	testopiaClient.productVersion = "unspecified"; // TODO: create if missing
	testopiaClient.build = "V0.9";
	testopiaClient.product = "TestopiaClient";
	testopiaClient.planId = 1;
	testopiaClient.environment = "Windows";

	testopiaClient.runSummary = "API TEST RUN";

	TestRegistry::getCurrentRegistry()->installPlugin(&testopiaClient);
	return testopiaClient.runRunAllTests(nArgc, ppArgv);
#endif
}
#endif /* TESTOPIA_CLIENT_TEST */
