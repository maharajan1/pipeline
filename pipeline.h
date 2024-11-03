#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector>
#include <memory>
#include "handler_context.h"
#include "handler.h"

class PipelineBase : public std::enable_shared_from_this<PipelineBase> {
public:
    virtual ~PipelineBase() = default;

    virtual void finalize() = 0;

    template <class H>
    PipelineBase& addBack(H&& handler);

    template <class H>
    PipelineBase& addBack(std::shared_ptr<H> handler);

protected:
    std::vector<std::shared_ptr<PipelineContext>> ctxs_;
    std::vector<PipelineContext*> inCtxs_;

private:
    template <class Context>
    PipelineBase& addHelper(std::shared_ptr<Context>&& ctx, bool front);
};

template <class H>
PipelineBase& PipelineBase::addBack(std::shared_ptr<H> handler) {
  return addHelper(
      std::make_shared<ContextImpl<H>>(shared_from_this(), std::move(handler)), false);
}

template <class H>
PipelineBase& PipelineBase::addBack(H&& handler) {
  return addBack(std::make_shared<H>(std::forward<H>(handler)));
}

template <class Context>
PipelineBase& PipelineBase::addHelper(
    std::shared_ptr<Context>&& ctx,
    bool front) {
  ctxs_.insert(front ? ctxs_.begin() : ctxs_.end(), ctx);
  inCtxs_.insert(front ? inCtxs_.begin() : inCtxs_.end(), ctx.get());
  return *this;
}

template <class R, class W>
class Pipeline : public PipelineBase {
 public:
  using Ptr = std::shared_ptr<Pipeline>;

  static Ptr create() {
    return std::shared_ptr<Pipeline>(new Pipeline());
  }

  ~Pipeline() override;

  template <class T = R>
  void handle(R msg);

  void finalize() override;

 protected:
  Pipeline();
  explicit Pipeline(bool isStatic);

 private:
  Link<R>* front_{nullptr};
};

template <class R, class W>
Pipeline<R, W>::Pipeline() {}

template <class R, class W>
Pipeline<R, W>::~Pipeline() {
}

template <class R, class W>
template <class T>
void Pipeline<R, W>::handle(R msg) {
  front_->handle(std::forward<R>(msg));
}

template <class R, class W>
void Pipeline<R, W>::finalize() {
  front_ = nullptr;
  if (!inCtxs_.empty()) {
    front_ = dynamic_cast<Link<R>*>(inCtxs_.front());
    for (size_t i = 0; i < inCtxs_.size() - 1; i++) {
      inCtxs_[i]->setNextIn(inCtxs_[i + 1]);
    }
    inCtxs_.back()->setNextIn(nullptr);
  }

  for (auto it = ctxs_.rbegin(); it != ctxs_.rend(); it++) {
    (*it)->attachPipeline();
  }
}

#endif
