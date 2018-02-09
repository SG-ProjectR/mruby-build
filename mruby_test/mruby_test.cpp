// mruby_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stack>

typedef std::string str;
str ReadFile(const str& path)
{
	FILE* fp = nullptr;
	fopen_s(&fp, path.c_str(), "r");
	if (fp)
	{
		auto buf = new char[30000];
		auto read = fread_s(buf, 30000, 1, 30000, fp);
		buf[read] = '\0';
		auto ret = str(buf);
		delete[] buf;
		return ret;
	}
	return str();
}

void load_file(mrb_state* mrb, const str& path)
{
	auto c = mrbc_context_new(mrb);
	c->capture_errors = TRUE;
	c->dump_result = FALSE;
	mrbc_filename(mrb, c, path.c_str());

	auto parse = mrb_parse_string(mrb, ReadFile(path).c_str(), c);
	if (!parse)
		return;
	if (!parse->tree || parse->nerr)
		return;
	auto proc = mrb_generate_code(mrb, parse);
	if (proc && proc->body.irep)
	{
		;
	}
	else
	{
		return;
	}
	uint8_t* irepBuffer = nullptr;
	size_t irepLen = 0;
	auto dumpResult = mrb_dump_irep(mrb, proc->body.irep, DUMP_DEBUG_INFO | DUMP_ENDIAN_BIG, &irepBuffer, &irepLen);

	if (dumpResult != MRB_DUMP_OK)
		return;

	//auto ret = mrb_load_string_cxt(mrb, ReadFile(path).c_str(), c);
	auto ret = mrb_load_irep_cxt(mrb, irepBuffer, c);
	if (mrb->exc)
	{
		if (mrb_undef_p(ret))
		{
			std::cout << "undef" << std::endl;
			mrb_p(mrb, mrb_obj_value(mrb->exc));
		}
		else
		{
			std::cout << "mrb_print_error" << std::endl;
			mrb_print_error(mrb);
		}
		//mrb->exc = nullptr;
	}
	mrbc_context_free(mrb, c);
}

void print_backtrace(mrb_state *mrb, mrb_value backtrace)
{
	if (!mrb_array_p(backtrace)) return;
	int i, n = RARRAY_LEN(backtrace) - 1;
	if (n == 0) return;
	fprintf(stdout, "trace:\n");
	//UE_LOG(LogRubyScript, Error, TEXT("trace:"));
	for (i = 0; i<n; i++) {
		mrb_value entry = RARRAY_PTR(backtrace)[i];

		if (mrb_string_p(entry)) {
			fprintf(stdout, "\t[%d] %.*s\n", i, (int)RSTRING_LEN(entry), RSTRING_PTR(entry));
			//UE_LOG(LogRubyScript, Error, TEXT("\t[%d] %s"), i, *FString(RSTRING_PTR(entry)));
		}
	}
}

mrb_value get_stack(mrb_state *mrb, mrb_value backtrace)
{
	auto val = mrb_get_backtrace(mrb);
	print_backtrace(mrb, val);
	return mrb_nil_value();
}

using namespace std;

int main()
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);


	while (true)
	{
		cout << "begin" << endl;
		getc(stdin);

		stack<mrb_state*> states;

		for (auto i = 0; i <1000; ++i)
		{
			auto mrb = mrb_open();
			states.push(mrb);
			//mrb_define_method(mrb, mrb->kernel_module, "printStack", get_stack, MRB_ARGS_REQ(1));
			//load_file(mrb, "a.mrb");
			//mrb_close(mrb);
			cout << i << endl;
		}
		cout << "created" << endl;
		getc(stdin);

		while (!states.empty())
		{
			auto mrb = states.top();
			states.pop();
			mrb_close(mrb);
		}
		//_CrtDumpMemoryLeaks();
	}

	cout << "end" << endl;
	getc(stdin);
	return 0;
}

