#include "ayan/ayan.h"

using namespace ayan;

int main(int argc, char **argv) {
  if (argc != 3) {
    fmt::print("usuage: {0} <WebSocket Server IP> <Port>\n", argv[0]);
    ::exit(0);
  }

  util::clear_screen();
  fmt::print("hello ayan! \n");

  const char *ip   = argv[1];
  int         port = std::atoi(argv[2]);

  auto ayan = //
      BotBuilder::from(Envir::global())
          .connect(ip, port)
          .with_qid(2821006329)
          .with_name("Ayan")
          .build();

  ayan->run();

  util::block_here();
  return 0;
}