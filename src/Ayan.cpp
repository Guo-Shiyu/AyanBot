#include "Ayan.h"

using namespace ayan;

int main(int argc, char **argv)
{
	std::system("chcp 65001 & cls");     

	auto env = Env::from()
				   .with_name("Global")
				   .with_thread_num(1)
				   .init();

	auto bot = Bot::from(env)
				   .connect("127.0.0.1", "6700")
				   .with_name("Ayan")
				   .init();

	bot->start();

	block_here();
	return 0;
}