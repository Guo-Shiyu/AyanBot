#include "ayan/ayan.h"
#include "ayan/core/bot.h"
#include "ayan/core/service.h"
#include "ayan/utils/util.h"

using namespace ayan;

int main(int argc, char **argv) {
  if (argc != 3) {
    fmt::print("usuage: {0} <WebSocket Server IP> <Port>\n", argv[0]);
    ::exit(0);
  }

  util::clear_screen();

  const char* ip = argv[1];
  int port = std::atoi(argv[2]);

  fmt::print("hello ayan! \n");

  auto &env = Envir::global();

  auto ayan = BotBuilder::from(env)
    .connect(ip, port)
    .with_qid(2821006329)
    .build();

  ayan->subscribe<ServiceSecheduler>().run();

  util::block_here();
  return 0;
}