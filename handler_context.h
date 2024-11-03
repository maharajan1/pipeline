#ifndef HANDLER_CONTEXT_H
#define HANDLER_CONTEXT_H

#include <string>
#include <sstream>
#include "pipeline.h"

class PipelineBase;

template <class In>
class Link {
 public:
  virtual ~Link() = default;
  virtual void handle(In msg) = 0;
};

template <class In>
class HandlerContext {
 public:
  virtual ~HandlerContext() = default;

  virtual void fireHandle(In msg) = 0;

  virtual PipelineBase* getPipeline() = 0;
  virtual std::shared_ptr<PipelineBase> getPipelineShared() = 0;
};

class PipelineContext {
 public:
  virtual ~PipelineContext() = default;

  virtual void attachPipeline() = 0;
  virtual void detachPipeline() = 0;

  template <class H, class HandlerContext>
  void attachContext(H* handler, HandlerContext* ctx) {
    if (++handler->attachCount_ == 1) {
      handler->ctx_ = ctx;
    } else {
      handler->ctx_ = nullptr;
    }
  }

  template <class H, class HandlerContext>
  void detachContext(H* handler, HandlerContext* /*ctx*/) {
    if (handler->attachCount_ >= 1) {
      --handler->attachCount_;
    }
    handler->ctx_ = nullptr;
  }

  virtual void setNextIn(PipelineContext* ctx) = 0;
};


template <class H, class Context>
class ContextImplBase : public PipelineContext {
 public:
  ~ContextImplBase() override = default;

  H* getHandler() {
    return handler_.get();
  }

  void initialize(
      std::weak_ptr<PipelineBase> pipeline,
      std::shared_ptr<H> handler) {
    pipelineWeak_ = pipeline;
    pipelineRaw_ = pipeline.lock().get();
    handler_ = std::move(handler);
  }

  void attachPipeline() override {
    if (!attached_) {
      this->attachContext(handler_.get(), impl_);
      handler_->attachPipeline(impl_);
      attached_ = true;
    }
  }

  void detachPipeline() override {
    handler_->detachPipeline(impl_);
    attached_ = false;
    this->detachContext(handler_.get(), impl_);
  }

  void setNextIn(PipelineContext* ctx) override {
    if (!ctx) {
      nextIn_ = nullptr;
      return;
    }
    auto nextIn = dynamic_cast<Link<typename H::rout>*>(ctx);
    if (nextIn) {
      nextIn_ = nextIn;
    } else {
      std::ostringstream oss;
      oss << "inbound type mismatch after " << typeid(H).name();
      throw std::invalid_argument(oss.str());
    }
  }

 protected:
  Context* impl_;
  std::weak_ptr<PipelineBase> pipelineWeak_;
  PipelineBase* pipelineRaw_;
  std::shared_ptr<H> handler_;
  Link<typename H::rout>* nextIn_{nullptr};

 private:
  bool attached_{false};
};

template <class H>
class ContextImpl
    : public HandlerContext<typename H::rout>,
      public Link<typename H::rin>,
      public ContextImplBase<H, HandlerContext<typename H::rout>> {
 public:
  typedef typename H::rin Rin;
  typedef typename H::rout Rout;

  explicit ContextImpl(
      std::weak_ptr<PipelineBase> pipeline,
      std::shared_ptr<H> handler) {
    this->impl_ = this;
    this->initialize(pipeline, std::move(handler));
  }

  // For StaticPipeline
  ContextImpl() {
    this->impl_ = this;
  }

  ~ContextImpl() override = default;

  // HandlerContext overrides
  void fireHandle(Rout msg) override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->handle(std::forward<Rout>(msg));
    } else {
      std::cout << "reached end of pipeline" << std::endl;
    }
  }

  PipelineBase* getPipeline() override {
    return this->pipelineRaw_;
  }

  std::shared_ptr<PipelineBase> getPipelineShared() override {
    return this->pipelineWeak_.lock();
  }

  // Link overrides
  void handle(Rin msg) override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->handle(this, std::forward<Rin>(msg));
  }
};

#endif
