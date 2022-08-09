#include "ayan/ayan.h"

using namespace ayan;

#include <iostream>
#include <cuchar>


int main(int argc, char **argv)
{
	util::clear_screen();

	auto u32str = "这是中文"u32;
	std::cout << "hello, Ayan!" << std::endl;
	std::cout << (char*)u32str.data() << std::endl;

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