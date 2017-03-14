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

#include "TestopiaRpcClient.h"

TestopiaRpcClient::TestopiaRpcClient(ulxr::CppString & user,
		ulxr::CppString & pass, ulxr::CppString & host, unsigned port,
		unsigned to_sec) {
	this->host = host;
	this->user = user;
	this->pass = pass;
	this->port = port;
	this->to_sec = to_sec;
}
TestopiaRpcClient::~TestopiaRpcClient() {
}

const char* TestopiaRpcClient::GetVersion()
{
    return TESTOPIARPCCLIENT_VERSION;
}

ulxr::MethodResponse TestopiaRpcClient::Call(const ulxr::Char *name,
		Struct& xmlRpcCallParam) {
#ifdef DEBUG_PRINTF
	printf("TestopiaRpcClient::Call(%s, ...)\n", name);
#endif
	std::auto_ptr<ulxr::TcpIpConnection> conn;
#ifdef _MSC_VER
	{
		std::auto_ptr<ulxr::TcpIpConnection> temp(new ulxr::TcpIpConnection (false, host, port));
		conn = temp;
	}
#else
	conn.reset(new ulxr::TcpIpConnection(false, host, port));
#endif

	ulxr::HttpProtocol prot(conn.get());
	prot.setAcceptCookies(true);
	conn->setTimeout(to_sec);
	ulxr::Requester client(&prot);

	prot.open();
	prot.setMessageAuthentication(user, pass);

	ulxr::MethodCall xmlRpcCall(name);
	xmlRpcCall.addParam(xmlRpcCallParam);
	return client.call(xmlRpcCall, ULXR_PCHAR("/xmlrpc.cgi"));
}

int TestopiaRpcClient::ProductGetIdByName(const ulxr::Char * name) {
	Struct xmlRpcCallParam;
	ulxr::MethodResponse resp;
	int productId = 0;

	RpcString nameString;
	nameString.setString(name);
	xmlRpcCallParam.addMember(ULXR_PCHAR("name"), nameString);

	resp = Call(ULXR_PCHAR("TestopiaProduct.get"), xmlRpcCallParam);
#if 0
	ULXR_COUT << resp.getXml() << std::endl;
	<?xml version="1.0" encoding="utf-8"?><methodResponse><params><param><value><struct><member><name>allows_unconfirmed</name><value><i4>1</i4></value></member><member><name>classification_id</name><value><i4>1</i4></value></member><member><name>defaultmilestone</name><value><string>---</string></value></member><member><name>description</name><value><string>This is a test product. This ought to be blown away and replaced with real stuff in a finished installation of bugzilla.</string></value></member><member><name>id</name><value><i4>1</i4></value></member><member><name>isactive</name><value><i4>1</i4></value></member><member><name>name</name><value><string>TestProduct</string></value></member></struct></value></param></params></methodResponse>
#endif

	Struct respStruct = resp.getResult();

	Integer intProductId = respStruct.getMember("id");
	productId = intProductId.getInteger();

	return productId;
}

int TestopiaRpcClient::EnvironmentGetIdByName(const ulxr::Char * name) {
	Struct xmlRpcCallParam;
	ulxr::MethodResponse resp;
	int envId = 0;

	RpcString nameString;
	nameString.setString(name);
	xmlRpcCallParam.addMember(ULXR_PCHAR("name"), nameString);

	resp = Call(ULXR_PCHAR("Environment.list"), xmlRpcCallParam);
#if 0
	ULXR_COUT << resp.getXml() << std::endl;
	<?xml version="1.0" encoding="utf-8"?><methodResponse><params><param><value><array><data><value><struct><member><name>environment_id</name><value><i4>1</i4></value></member><member><name>isactive</name><value><i4>1</i4></value></member><member><name>name</name><value><string>Linux_x86</string></value></member><member><name>product_id</name><value><i4>1</i4></value></member></struct></value></data></array></value></param></params></methodResponse>
#endif
	Array respArray = resp.getResult();
	for (unsigned i = 0; i < respArray.size(); i++) {
		Struct respStruct = respArray.getItem(i);
		RpcString respName = respStruct.getMember("name");
		if (0 == respName.getString().compare(name)) {
			Integer intEnvId = respStruct.getMember("environment_id");
			envId = intEnvId.getInteger();
			break;
		}
	}
	return envId;
}

// name=>'Build '. time(), product_id=>1, isactive=>0, description=> 'API Test Build - IGNORE'
int TestopiaRpcClient::BuildCreate(const ulxr::Char * name, int productId,
		int isActive, const ulxr::Char * description) {
	Struct xmlRpcCallParam;
	ulxr::MethodResponse resp;
	int buildId = 0;

	RpcString stringBuildName;
	RpcString stringDescription;
	stringBuildName.setString(name);
	stringDescription.setString(description);
	xmlRpcCallParam.addMember(ULXR_PCHAR("name"), stringBuildName);
	xmlRpcCallParam.addMember(ULXR_PCHAR("product_id"), Integer(productId));
	xmlRpcCallParam.addMember(ULXR_PCHAR("isactive"), Integer(isActive));
	xmlRpcCallParam.addMember(ULXR_PCHAR("description"), stringDescription);

	resp = Call(ULXR_PCHAR("Build.create"), xmlRpcCallParam);
#if 0
	ULXR_COUT << resp.getXml() << std::endl;
	<?xml version="1.0" encoding="utf-8"?><methodResponse><params><param><value><struct><member><name>build_id</name><value><i4>2</i4></value></member><member><name>description</name><value><string>TEST(TestopiaRpcClient, TestBuildCreate)</string></value></member><member><name>isactive</name><value><i4>1</i4></value></member><member><name>milestone</name><value><string>---</string></value></member><member><name>name</name><value><string>V0.0.1.02</string></value></member><member><name>product_id</name><value><i4>1</i4></value></member></struct></value></param></params></methodResponse>
#endif

	Struct respStruct = resp.getResult();
	Integer intBuildId = respStruct.getMember("build_id");
	buildId = intBuildId.getInteger();
#ifdef DEBUG_PRINTF
    std::cout << "build id: " << buildId << std::endl;
#endif
	return buildId;
}

int TestopiaRpcClient::BuildGetIdByProductNameAndBuildName(
		const ulxr::Char * productName, const ulxr::Char * buildName) {
	Struct xmlRpcCallParam;
	ulxr::MethodResponse resp;
	int buildId = 0;

	RpcString productNameString;
	productNameString.setString(productName);
	xmlRpcCallParam.addMember(ULXR_PCHAR("name"), productNameString);

	resp = Call(ULXR_PCHAR("TestopiaProduct.get_builds"), xmlRpcCallParam);
#if 0
	ULXR_COUT << resp.getXml() << std::endl;
	<?xml version="1.0" encoding="utf-8"?><methodResponse><params><param><value><array><data><value><struct><member><name>build_id</name><value><i4>1</i4></value></member><member><name>description</name><value><string></string></value></member><member><name>isactive</name><value><i4>1</i4></value></member><member><name>milestone</name><value><string>---</string></value></member><member><name>name</name><value><string>V0.0.1.01</string></value></member><member><name>product_id</name><value><i4>1</i4></value></member></struct></value></data></array></value></param></params></methodResponse>
#endif

	Array respArray = resp.getResult();
	for (unsigned i = 0; i < respArray.size(); i++) {
		Struct respStruct = respArray.getItem(i);
		RpcString respName = respStruct.getMember(ULXR_PCHAR("name"));
		if (0 == respName.getString().compare(buildName)) {
			Integer intBuildId = respStruct.getMember(ULXR_PCHAR("build_id"));
#ifdef DEBUG_PRINTF
			std::cout << "name: " << respName.getString() << ", build id: "
					<< intBuildId.getInteger() << std::endl;
#endif

			buildId = intBuildId.getInteger();
			break;
		}
	}
#ifdef DEBUG_PRINTF
    std::cout << "build id: " << buildId << std::endl;
#endif
	return buildId;
}

int TestopiaRpcClient::TestCaseGetIdByPlanIdAndSummary(int testPlanId,
		const ulxr::Char * summary, int isAutomated) {
	Struct xmlRpcCallParam;
	ulxr::MethodResponse resp;
	int testCaseId = 0;

	if (0 != testPlanId) {
		xmlRpcCallParam.addMember(ULXR_PCHAR("id"), Integer(testPlanId));
		resp = Call(ULXR_PCHAR("TestPlan.get_test_cases"), xmlRpcCallParam);
	} else {
		// TODO: add support for >1000 results (iterate pages)
		xmlRpcCallParam.addMember(ULXR_PCHAR("pagesize"), Integer(1000));
		xmlRpcCallParam.addMember(ULXR_PCHAR("page"), Integer(0));
		xmlRpcCallParam.addMember(ULXR_PCHAR("isautomated"),
				Integer(isAutomated));
		resp = Call(ULXR_PCHAR("TestCase.list"), xmlRpcCallParam);
	}

	Array respArray = resp.getResult();
	for (unsigned i = 0; i < respArray.size(); i++) {
		Struct respStruct = respArray.getItem(i);
		RpcString respSummary = respStruct.getMember("summary");
		if (0 == respSummary.getString().compare(summary)) {
			Integer intTestCaseId = respStruct.getMember("case_id");
#ifdef DEBUG_PRINTF
            std::cout << "summary: " << respSummary.getString()
					<< ", test case id: " << intTestCaseId.getInteger()
					<< std::endl;
#endif
			testCaseId = intTestCaseId.getInteger();
			break;
		}

	}
#ifdef DEBUG_PRINTF
    std::cout << "test case id: " << testCaseId << std::endl;
#endif
	return testCaseId;
}

int TestopiaRpcClient::TestCaseCreateByPlanIdAndSummary(int testPlanId,
		const ulxr::Char * summary, const ulxr::Char * status,
		const ulxr::Char * category, const ulxr::Char * priority) {
#ifdef DEBUG_PRINTF
    std::cout << "TestCaseCreateByPlanIdAndSummary: plan " << testPlanId
			<< ", summary " << summary << std::endl;
#endif
	Struct xmlRpcCallParam;
	ulxr::MethodResponse resp;
	int testCaseId = 0;

	RpcString statusString;
	RpcString categoryString;
	RpcString priorityString;
	RpcString summaryString;

	statusString.setString(status);
	categoryString.setString(category);
	priorityString.setString(priority);
	summaryString.setString(summary);
	xmlRpcCallParam.addMember(ULXR_PCHAR("status"), statusString);
	xmlRpcCallParam.addMember(ULXR_PCHAR("category"), categoryString);
	xmlRpcCallParam.addMember(ULXR_PCHAR("priority"), priorityString);
	xmlRpcCallParam.addMember(ULXR_PCHAR("summary"), summaryString);

	Array arr;
	arr.addItem(Integer(testPlanId));
	xmlRpcCallParam.addMember(ULXR_PCHAR("plans"), arr);

	resp = Call(ULXR_PCHAR("TestCase.create"), xmlRpcCallParam);
#if 0
	ULXR_COUT << resp.getXml() << std::endl;
	<?xml version="1.0" encoding="utf-8"?><methodResponse><params><param><value><struct><member><name>author_id</name><value><i4>1</i4></value></member><member><name>case_id</name><value><i4>5</i4></value></member><member><name>case_status_id</name><value><i4>2</i4></value></member><member><name>category_id</name><value><i4>1</i4></value></member><member><name>creation_date</name><value><string>2015-08-05 12:13:00</string></value></member><member><name>isautomated</name><value><i4>0</i4></value></member><member><name>priority_id</name><value><i4>4</i4></value></member><member><name>summary</name><value><string>ConnectionStateMachineTestGroup, TestDataRequested</string></value></member><member><name>version</name><value><i4>1</i4></value></member></struct></value></param></params></methodResponse>
#endif

	Struct respStruct = resp.getResult();
	Integer intTestCaseId = respStruct.getMember("case_id");
	testCaseId = intTestCaseId.getInteger();
#ifdef DEBUG_PRINTF
    std::cout << "test case id: " << testCaseId << std::endl;
#endif
    return testCaseId;
}

int TestopiaRpcClient::TestRunCreate(int plan_id,
		const ulxr::Char * environment, const ulxr::Char * build,
		const ulxr::Char * summary, const ulxr::Char * manager_id,
		const ulxr::Char * product_version, int status, int & envId,
		int & buildId, int & managerId) {
	int run_id = 0;
	try {
#ifdef DEBUG_PRINTF
        std::cout << "TestRunCreate: " << summary << std::endl;
#endif
		Struct xmlRpcCallParam;
		xmlRpcCallParam.addMember(ULXR_PCHAR("plan_id"), Integer(plan_id));

		RpcString environmentString;
		RpcString buildString;
		RpcString summaryString;
		RpcString managerIdString;
		RpcString productVersionString;

		environmentString.setString(environment);
		buildString.setString(build);
		managerIdString.setString(manager_id);
		summaryString.setString(summary);
		productVersionString.setString(product_version);
		xmlRpcCallParam.addMember(ULXR_PCHAR("environment"), environmentString);
		xmlRpcCallParam.addMember(ULXR_PCHAR("build"), buildString);
		xmlRpcCallParam.addMember(ULXR_PCHAR("summary"), summaryString);
		xmlRpcCallParam.addMember(ULXR_PCHAR("manager_id"), managerIdString);
		xmlRpcCallParam.addMember(ULXR_PCHAR("product_version"),
				productVersionString);

		xmlRpcCallParam.addMember(ULXR_PCHAR("status"), Integer(status));

		ulxr::MethodResponse resp = Call(ULXR_PCHAR("TestRun.create"),
				xmlRpcCallParam);
#if 0
		ULXR_COUT << resp.getXml() << std::endl;
		<?xml version="1.0" encoding="utf-8"?><methodResponse><params><param><value><struct><member><name>build_id</name><value><i4>1</i4></value></member><member><name>environment_id</name><value><i4>1</i4></value></member><member><name>manager_id</name><value><i4>1</i4></value></member><member><name>plan_id</name><value><i4>1</i4></value></member><member><name>plan_text_version</name><value><i4>1</i4></value></member><member><name>product_version</name><value><string>unspecified</string></value></member><member><name>run_id</name><value><i4>3</i4></value></member><member><name>start_date</name><value><string>2015-08-05 07:34:37</string></value></member><member><name>summary</name><value><string>API TEST RUN</string></value></member></struct></value></param></params></methodResponse>
#endif
		Struct respStruct = resp.getResult();
		Integer intRunId = respStruct.getMember("run_id");
		Integer intEnvId = respStruct.getMember("environment_id");
		Integer intBuildId = respStruct.getMember("build_id");
		Integer intManagerId = respStruct.getMember("manager_id");
		run_id = intRunId.getInteger();
		envId = intEnvId.getInteger();
		buildId = intBuildId.getInteger();
		managerId = intManagerId.getInteger();
	}

	catch (ulxr::Exception &ex) {
		ULXR_COUT << ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()) << std::endl;
		return 1;
	}
	catch (...) {
		ULXR_COUT << ULXR_PCHAR("unknown Error occured.\n");
		return 1;
	}
#ifdef DEBUG_PRINTF
	ULXR_COUT << ULXR_PCHAR("TestRunCreate done. run_id ") << run_id << std::endl;
#endif
	return run_id;
}

void TestopiaRpcClient::TestRunAddCase(int testCaseId, int runId) {
#ifdef DEBUG_PRINTF
    std::cout << "TestRunAddCase: case " << testCaseId << ", run " << runId
			<< std::endl;
#endif
	Array arrCaseIds;
	Array arrRunIds;
	arrCaseIds.addItem(Integer(testCaseId));
	arrRunIds.addItem(Integer(runId));

	Struct xmlRpcCallParam;
	xmlRpcCallParam.addMember(ULXR_PCHAR("case_ids"), arrCaseIds);
	xmlRpcCallParam.addMember(ULXR_PCHAR("run_ids"), arrRunIds);
	Call(ULXR_PCHAR("TestRun.add_cases"), xmlRpcCallParam);
}

void TestopiaRpcClient::TestRunStatusUpdate(int run_id, int status)
{
	Struct xmlRpcCallParam;
	xmlRpcCallParam.addMember(ULXR_PCHAR("id"), Integer(run_id));
	xmlRpcCallParam.addMember(ULXR_PCHAR("status"), Integer(status));
	Call(ULXR_PCHAR("TestRun.update"), xmlRpcCallParam);
}

int TestopiaRpcClient::TestCaseRunUpdate(const ulxr::Char * summary, int status,
		int testPlanId, int testRunId, int buildId, int envId, int testCaseId) {

#ifdef DEBUG_PRINTF
    std::cout << "Searching summary: " << summary << std::endl;
#endif
	if (0 == testCaseId)
		testCaseId = TestCaseGetIdByPlanIdAndSummary(testPlanId, summary);

	if (0 == testCaseId)
		testCaseId = TestCaseGetIdByPlanIdAndSummary(0, summary);

	if (0 == testCaseId) {
		testCaseId = TestCaseCreateByPlanIdAndSummary(testPlanId, summary);
		TestRunAddCase(testCaseId, testRunId);
	}

	int testCaseRunId = 0;
	{
		Struct xmlRpcCallParam;
		xmlRpcCallParam.addMember(ULXR_PCHAR("case_id"), Integer(testCaseId));
		xmlRpcCallParam.addMember(ULXR_PCHAR("run_id"), Integer(testRunId));
		xmlRpcCallParam.addMember(ULXR_PCHAR("build_id"), Integer(buildId));
		xmlRpcCallParam.addMember(ULXR_PCHAR("env_id"), Integer(envId));
		ulxr::MethodResponse resp = Call(ULXR_PCHAR("TestCaseRun.get"),
				xmlRpcCallParam);
#if 0
		ULXR_COUT << "TestCaseRun.get" << std::endl << resp.getXml() << std::endl;
		<?xml version="1.0" encoding="utf-8"?><methodResponse><params><param><value><struct><member><name>assignee</name><value><i4>1</i4></value></member><member><name>build_id</name><value><i4>1</i4></value></member><member><name>case_id</name><value><i4>3</i4></value></member><member><name>case_run_id</name><value><i4>28</i4></value></member><member><name>case_run_status_id</name><value><i4>1</i4></value></member><member><name>case_text_version</name><value><i4>1</i4></value></member><member><name>environment_id</name><value><i4>1</i4></value></member><member><name>iscurrent</name><value><i4>1</i4></value></member><member><name>priority_id</name><value><i4>3</i4></value></member><member><name>run_id</name><value><i4>14</i4></value></member></struct></value></param></params></methodResponse>
#endif

		try {
			Struct respStruct = resp.getResult();
			Integer intTestCaseRunId = respStruct.getMember("case_run_id");
			testCaseRunId = intTestCaseRunId.getInteger();
		} catch (...) {
		}
	}

	if (0 == testCaseRunId) {
		Struct xmlRpcCallParam;
		xmlRpcCallParam.addMember(ULXR_PCHAR("case_id"), Integer(testCaseId));
		xmlRpcCallParam.addMember(ULXR_PCHAR("run_id"), Integer(testRunId));
		xmlRpcCallParam.addMember(ULXR_PCHAR("build_id"), Integer(buildId));
		xmlRpcCallParam.addMember(ULXR_PCHAR("env_id"), Integer(envId));
		ulxr::MethodResponse resp = Call(ULXR_PCHAR("TestCaseRun.create"),
				xmlRpcCallParam);
#ifdef DEBUG_PRINTF
        ULXR_COUT << "TestCaseRun.create" << std::endl << resp.getXml() << std::endl;
#endif
#if 0
		<?xml version="1.0" encoding="utf-8"?><methodResponse><params><param><value><struct><member><name>build_id</name><value><i4>1</i4></value></member><member><name>environment_id</name><value><i4>1</i4></value></member><member><name>manager_id</name><value><i4>1</i4></value></member><member><name>plan_id</name><value><i4>1</i4></value></member><member><name>plan_text_version</name><value><i4>1</i4></value></member><member><name>product_version</name><value><string>unspecified</string></value></member><member><name>run_id</name><value><i4>3</i4></value></member><member><name>start_date</name><value><string>2015-08-05 07:34:37</string></value></member><member><name>summary</name><value><string>API TEST RUN</string></value></member></struct></value></param></params></methodResponse>
#endif
	}

	{
		Array arr;
		arr.addItem(Integer(testCaseRunId));
		Struct xmlRpcCallParam;
		xmlRpcCallParam.addMember(ULXR_PCHAR("ids"), arr);
		xmlRpcCallParam.addMember(ULXR_PCHAR("status"), Integer(status));
		ulxr::MethodResponse resp = Call(ULXR_PCHAR("TestCaseRun.update"),
				xmlRpcCallParam);
#if 0
		ULXR_COUT << "TestCaseRun.update" << std::endl << resp.getXml() << std::endl;
		<?xml version="1.0" encoding="utf-8"?><methodResponse><params><param><value><struct><member><name>build_id</name><value><i4>1</i4></value></member><member><name>environment_id</name><value><i4>1</i4></value></member><member><name>manager_id</name><value><i4>1</i4></value></member><member><name>plan_id</name><value><i4>1</i4></value></member><member><name>plan_text_version</name><value><i4>1</i4></value></member><member><name>product_version</name><value><string>unspecified</string></value></member><member><name>run_id</name><value><i4>3</i4></value></member><member><name>start_date</name><value><string>2015-08-05 07:34:37</string></value></member><member><name>summary</name><value><string>API TEST RUN</string></value></member></struct></value></param></params></methodResponse>
#endif
	}
#ifdef DEBUG_PRINTF
    ULXR_COUT << ULXR_PCHAR("TestCaseRunUpdate done. case_id " << testCaseId << ", case_run_id " << testCaseRunId << ", status " << status << std::endl);
#endif
	return testCaseId;
}
