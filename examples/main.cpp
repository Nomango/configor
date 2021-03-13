// Copyright (c) 2021 Nomango

#include <iostream>
#include "handler.h"

int main(int argc, char** argv)
{
	istringstream req("{\n    \"user_id\": 10001\n}\n");
	ostringstream resp;

	GetUserInfoHandler handler;
	handler.POST(req, resp);

	std::cout << "request: \n" << req.str() << std::endl;
	std::cout << "response: \n" << resp.str() << std::endl;
	return 0;
}
