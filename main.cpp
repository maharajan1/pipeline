#include <iostream>
#include <memory>
#include "pipeline.h"

struct Handler1 : public Handler<int, std::string> {
  void handle(Context* ctx, int msg) override {
    std::cout << "Handler1 : " << msg << std::endl;
    ctx->fireHandle(std::to_string(msg));
  }
};

struct Handler2 : public Handler<std::string> {
  void handle(Context* ctx, std::string msg) override {
    std::cout << "Handler2 : " << msg << std::endl;
    ctx->fireHandle(msg + std::string(" Handler2"));
  }
};

struct Handler3 : public Handler<std::string> {
  void handle(Context* ctx, std::string msg) override {
    std::cout << "Handler3 : " << msg << std::endl;
    ctx->fireHandle(msg + std::string(" Handler3"));
  }
};

int main() {
  auto p = Pipeline<int, std::string>::create();
  p->addBack(Handler1());
  p->addBack(Handler2());
  p->addBack(Handler3());
  p->finalize();
  p->handle(2);
  return 0;
}
