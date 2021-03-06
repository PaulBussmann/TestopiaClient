#include <iostream>
#include <sstream>
#include <string>
#include <regex>

#include "TestopiaRpcClient.h"

using namespace std;

// Testopia data
static std::string strProduct = "";
static std::string strProductVersion = "";
static std::string strBuild = "";
static std::string strBuildDescription = "";
static std::string strEnvironment = "---";
static std::string strRunSummary = "---";
static std::string strRunManagerName = "admin@localhost.localdomain";

static int runManagerId = 0;
static int productId = 0;
static int buildId = 0;
static int envId = 0;
static int planId = 0;
static int runId = 0;

// Testopia connection parameters
static ulxr::CppString strUser = ULXR_PCHAR("admin@localhost.localdomain");
static ulxr::CppString strPassword = ULXR_PCHAR("password");
static ulxr::CppString strHost = ULXR_PCHAR("localhost");
static unsigned port = 80;
static unsigned to_sec = 30;

// Testopia connection handle
static TestopiaRpcClient* rpcClient = NULL;

void TestRunInit() {
    if (0 == productId)
        productId = rpcClient->ProductGetIdByName(strProduct.c_str());

    if (0 == envId)
        envId = rpcClient->EnvironmentGetIdByName(strEnvironment.c_str());

    if (0 == envId)
        envId = rpcClient->EnvironmentCreate(productId, strEnvironment.c_str(), 1);

    if (0 == runManagerId)
        runManagerId = rpcClient->TestopiaUserGetIdByLogin(strRunManagerName.c_str());

    // root@testopia:/var/www/html/Bugzilla/Version.pm
    // How to create version via HTTP?

    if (0 == buildId)
        buildId = rpcClient->BuildGetIdByProductNameAndBuildName(strProduct.c_str(), strBuild.c_str());

    if (0 == buildId)
        rpcClient->BuildCreate(strBuild.c_str(), productId, 0, strBuildDescription.c_str());

    if (0 == runId)
        runId = rpcClient->TestRunCreate(planId, strEnvironment.c_str(), strBuild.c_str(), strRunSummary.c_str(), strRunManagerName.c_str(), strProductVersion.c_str(), TestRunStatus_RUNNING,
            envId, buildId, runManagerId);
}

void TestRunStop() {
    if (0 == runId)
        TestRunInit();

    rpcClient->TestRunStatusUpdate(runId, TestRunStatus_STOPPED);
}

int TestCaseRunInit(const char* name) {
    int testCaseId = rpcClient->TestCaseGetIdByPlanIdAndSummary(planId, name);

    if (0 == runId)
        TestRunInit();

    if (0 == testCaseId)
        testCaseId = rpcClient->TestCaseGetIdByPlanIdAndSummary(0, name);

    if (0 == testCaseId)
        testCaseId = rpcClient->TestCaseCreateByPlanIdAndSummary(planId, name);

    if ((0 != runId) && (0 != testCaseId))
        rpcClient->TestRunAddCase(testCaseId, runId);

    return testCaseId;
}

void TestCaseRunUpdate(const char* name, int testCaseId, int status = (int)TestCaseStatus_IDLE) {
    rpcClient->TestCaseRunUpdate(name, status, planId, runId, buildId, envId, testCaseId);
}

bool equals(std::string const &a, std::string const &b) {
    return a.compare(b) == 0;
}

bool startsWith(std::string const &fullString, std::string const &starting) {
    return fullString.find(starting) == 0;
}

bool contains(std::string const &haystack, std::string const &needle) {
    return haystack.find(needle) < haystack.length();
}

bool endsWith(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length())
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    else
        return false;
}

void finalizeTest(const std::string & testname, int testCaseId, bool failure, long duration, const std::string & errorhint) {
    std::cout << "Test " << testname << " finished " << (failure ? "not " : "") << "ok in " << duration << " ms" << std::endl;
    std::cout << errorhint;

    // set last test finished success yes/no
    TestCaseRunUpdate(testname.c_str(), testCaseId, (failure ? (int)TestCaseStatus_FAILED:(int)TestCaseStatus_PASSED));

    // duration and errorhint currently not available
    // TODO:
    // add parameters date, note to "sub set_status {" in bugzilla\extensions\Testopia\lib\TestCaseRun.pm
    // add "sub update_detailed { " in bugzilla\extensions\Testopia\lib\WebService\TestCaseRun.pm
}

// read from stdin, update testopia and write to stdout
int main(int argc, char* argv[]) {

    // testopia context information from environment
    if (NULL != getenv("TESTOPIA_USER")) strUser = ULXR_PCHAR(getenv("TESTOPIA_USER"));
    if (NULL != getenv("TESTOPIA_PASSWORD")) strPassword = ULXR_PCHAR(getenv("TESTOPIA_PASSWORD"));
    if (NULL != getenv("TESTOPIA_HOST")) strHost = ULXR_PCHAR(getenv("TESTOPIA_HOST"));

    // testopia context information from command line
    for (int i = 1; i < argc; i++) {
        if (equals("-host", argv[i]))
            strHost = ULXR_PCHAR(argv[++i]);
        else if (equals("-manager", argv[i]))
            strRunManagerName = argv[++i];
        else if (equals("-product", argv[i]))
            strProduct = argv[++i];
        else if (equals("-version", argv[i]))
            strProductVersion = argv[++i];
        else if (equals("-build", argv[i]))
            strBuild = argv[++i];
        else if (equals("-environment", argv[i]))
            strEnvironment = argv[++i];
        else if (equals("-user", argv[i]))
            strUser = argv[++i];
        else if (equals("-planid", argv[i]))
            planId = std::stoi(argv[++i]);
        else if ('-' == argv[i][0]) {
            std::cerr <<
            "Usage:" << std::endl <<
            "-manager <manager name>" << std::endl <<
            "-product <product name>" << std::endl <<
            "-version <version>" << std::endl <<
            "-build <build>" << std::endl <<
            "-environment <environment>" << std::endl <<
            "-user <user name>" << std::endl <<
            "-planid <plan ID>" << std::endl;
        }
    }

    // test run / test case data
    bool finished = false;
    std::string line;
    std::string testname = "";
    bool failure = false;
    long duration = 0;
    string errorhint = "";
    int testCaseId = 0;

    // connect to Testopia
    rpcClient = new TestopiaRpcClient(strUser, strPassword, strHost, port, to_sec);

    // Read CppUTest output from cmd line
    while (!finished) {
        line = "";
        std::getline(std::cin, line);  // read a line from std::cin into line

        // testopia context information from stdin
        if (startsWith(line, "PRODUCT("))
            strProduct = line.substr(strlen("PRODUCT("), line.find(")") - strlen("PRODUCT("));;
        if (startsWith(line, "VERSION("))
            strProductVersion = line.substr(strlen("VERSION("), line.find(")") - strlen("VERSION("));;
        if (startsWith(line, "BUILD("))
            strBuild = line.substr(strlen("BUILD("), line.find(")") - strlen("BUILD("));;
        if (startsWith(line, "ENVIRONMENT("))
            strEnvironment = line.substr(strlen("ENVIRONMENT("), line.find(")") - strlen("ENVIRONMENT("));;

        // test starting
        if (startsWith(line, "TEST(")) {

            // first test or next test (finalize previous)
            if (testname.length() > 0)
                finalizeTest(testname, testCaseId, failure, duration, errorhint);

            // next test: get name and reset info
            testname = line.substr(5, line.find(")") - 5);
            failure = false;
            duration = 0;
            errorhint = "";

            // set status running and create if missing
            testCaseId = TestCaseRunInit(testname.c_str());

            // set running
            TestCaseRunUpdate(testname.c_str(), testCaseId, TestCaseStatus_RUNNING);
        }

        // ignored tests cannot be executed. set to idle.
        if (startsWith(line, "IGNORE_TEST(")) {

            if (testname.length() > 0)
                finalizeTest(testname, testCaseId, failure, duration, errorhint);

            // ignored test name
            testname = line.substr(12, line.find(")") - 12);

            // create if missing
            testCaseId = TestCaseRunInit(testname.c_str());

            // set idle
            TestCaseRunUpdate(testname.c_str(), testCaseId, TestCaseStatus_IDLE);
            testname = "";
        }

        // failure in test
        if (contains(line, ": error: Failure in TEST(")) {
             failure = true;
             TestCaseRunUpdate(testname.c_str(), testCaseId, TestCaseStatus_FAILED);
        }

        // duration
        if (endsWith(line, " ms")) {
            std::regex duration_regex("(.* \\- )([0-9]+)( ms)");
            std::smatch duration_match;

            if (std::regex_match(line, duration_match, duration_regex) && (duration_match.size() == 4))
                duration = std::stoi(duration_match[2].str());

        }

        // test run finished (all cases tested)
        if (startsWith(line, "OK (") || startsWith(line, "Errors ("))
            finished = true;

        // failure but not all cases tested yet
        if (failure && !finished) {
            std::ostringstream errorhintstream;
            errorhintstream << errorhint << line << std::endl;
            errorhint = errorhintstream.str();
        }

        std::cout << line << std::endl;
    }

    if (testname.length() > 0)
        finalizeTest(testname, testCaseId, failure, duration, errorhint);

    // finish run
    TestRunStop();

    delete rpcClient;

    return 0;
}
