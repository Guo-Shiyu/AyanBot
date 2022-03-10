#include "Ayan.h"

using namespace ayan;

int main(int argc, char** argv) {
	std::system("chcp 65001 && cls");

	auto env = ayan::Env::from()
		.with_name("Global Environment")
		.with_thread_num(1)
		.init();

	auto bot = ayan::Bot::from(env)
		.connect("127.0.0.1", "6700")
		.with_name("Ayan")
		.init();

	bot->start();

	block_here();
	return 0;
}