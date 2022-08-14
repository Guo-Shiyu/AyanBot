#include "ayan/ayan.h"

using namespace ayan;

#include <iostream>

int main(int argc, char **argv)
{
	util::clear_screen();

	std::cout << "hello, Ayan!" << std::endl;

	// auto env = Env::from()
	// 			   .with_name("Global")
	// 			   .with_thread_num(1)
	// 			   .init();

	// auto bot = Bot::from(env)
	// 			   .connect("127.0.0.1", "6700")
	// 			   .with_name("Ayan")
	// 			   .init();

	// bot->start();

	util::block_here();
	return 0;
}