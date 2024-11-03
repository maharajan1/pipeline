#ifndef HANDLER_H
#define HANDLER_H

#include "handler_context.h"

template <class Context>
class HandlerBase {
 public:
  virtual ~HandlerBase() = default;

  virtual void attachPipeline(Context* /*ctx*/) {}
  virtual void detachPipeline(Context* /*ctx*/) {}

  Context* getContext() {
    if (attachCount_ != 1) {
      return nullptr;
    }
    CHECK(ctx_);
    return ctx_;
  }

 private:
  friend PipelineContext;
  uint64_t attachCount_{0};
  Context* ctx_{nullptr};
};

template <class Rin, class Rout = Rin>
class Handler : public HandlerBase<HandlerContext<Rout>> {
 public:
  typedef Rin rin;
  typedef Rout rout;
  typedef HandlerContext<Rout> Context;
  ~Handler() override = default;

  virtual void handle(Context* ctx, Rin msg) = 0;
};

#endif
