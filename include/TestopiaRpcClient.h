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

#ifndef TESTOPIARPCCLIENT_H_
#define TESTOPIARPCCLIENT_H_

#define TESTOPIARPCCLIENT_VERSION "0.9.2"

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <memory>
#include <cstring>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_log4j.h>

using namespace ulxr;

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

/// Testopia RPC access
class TestopiaRpcClient {
public:
	TestopiaRpcClient(ulxr::CppString & user, ulxr::CppString & pass,
			ulxr::CppString & host, unsigned port = 80, unsigned to_sec = 30);
	~TestopiaRpcClient();
    static const char* GetVersion(); 
	ulxr::MethodResponse Call(const ulxr::Char *name, Struct& xmlRpcCallParam);

    int TestopiaUserGetIdByLogin(const ulxr::Char * name);
	int ProductGetIdByName(const ulxr::Char * name);
	int TestCaseCreateByPlanIdAndSummary(int testPlanId, const ulxr::Char * summary,
			const ulxr::Char * status = "CONFIRMED",
			const ulxr::Char * category = "--default--",
			const ulxr::Char * priority = "Normal");
	int EnvironmentGetIdByName(const ulxr::Char * name);

	int BuildCreate(const ulxr::Char * name, int productId, int isActive, const ulxr::Char * description);

	int BuildGetIdByProductNameAndBuildName(const ulxr::Char * productName, const ulxr::Char * buildName);

	int TestCaseGetIdByPlanIdAndSummary(int testPlanId, const ulxr::Char * summary,
			int isAutomated = 1);
	void TestRunAddCase(int testCaseId, int runId);
	int TestCaseRunUpdate(const ulxr::Char * summary, int status, int testPlanId,
			int testRunId, int buildId, int envId, int testCaseId = 0);
	int TestRunCreate(int plan_id, const ulxr::Char * environment, const ulxr::Char * build,
			const ulxr::Char * summary, const ulxr::Char * manager_id,
			const ulxr::Char * product_version, int status, int & envId, int & buildId, int & managerId);
	void TestRunStatusUpdate(int run_id, int status);

	ulxr::CppString user;
	ulxr::CppString pass;

	ulxr::CppString host;
	unsigned port;
	unsigned to_sec;
};

#endif /* TESTOPIARPCCLIENT_H_ */
