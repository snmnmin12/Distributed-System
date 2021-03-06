// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: chat.proto

#include "chat.pb.h"
#include "chat.grpc.pb.h"

#include <grpc++/impl/codegen/async_stream.h>
#include <grpc++/impl/codegen/async_unary_call.h>
#include <grpc++/impl/codegen/channel_interface.h>
#include <grpc++/impl/codegen/client_unary_call.h>
#include <grpc++/impl/codegen/method_handler_impl.h>
#include <grpc++/impl/codegen/rpc_service_method.h>
#include <grpc++/impl/codegen/service_type.h>
#include <grpc++/impl/codegen/sync_stream.h>
namespace chatroom {

static const char* ChatService_method_names[] = {
  "/chatroom.ChatService/StartChat",
  "/chatroom.ChatService/Join",
  "/chatroom.ChatService/Leave",
  "/chatroom.ChatService/List",
  "/chatroom.ChatService/Send",
  "/chatroom.ChatService/SendAll",
  "/chatroom.ChatService/RecvAll",
};

std::unique_ptr< ChatService::Stub> ChatService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  std::unique_ptr< ChatService::Stub> stub(new ChatService::Stub(channel));
  return stub;
}

ChatService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_StartChat_(ChatService_method_names[0], ::grpc::RpcMethod::BIDI_STREAMING, channel)
  , rpcmethod_Join_(ChatService_method_names[1], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Leave_(ChatService_method_names[2], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_List_(ChatService_method_names[3], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Send_(ChatService_method_names[4], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_SendAll_(ChatService_method_names[5], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_RecvAll_(ChatService_method_names[6], ::grpc::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::ClientReaderWriter< ::chatroom::Message, ::chatroom::Response>* ChatService::Stub::StartChatRaw(::grpc::ClientContext* context) {
  return new ::grpc::ClientReaderWriter< ::chatroom::Message, ::chatroom::Response>(channel_.get(), rpcmethod_StartChat_, context);
}

::grpc::ClientAsyncReaderWriter< ::chatroom::Message, ::chatroom::Response>* ChatService::Stub::AsyncStartChatRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::ClientAsyncReaderWriter< ::chatroom::Message, ::chatroom::Response>::Create(channel_.get(), cq, rpcmethod_StartChat_, context, tag);
}

::grpc::Status ChatService::Stub::Join(::grpc::ClientContext* context, const ::chatroom::Request& request, ::chatroom::Response* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_Join_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::chatroom::Response>* ChatService::Stub::AsyncJoinRaw(::grpc::ClientContext* context, const ::chatroom::Request& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::ClientAsyncResponseReader< ::chatroom::Response>::Create(channel_.get(), cq, rpcmethod_Join_, context, request);
}

::grpc::Status ChatService::Stub::Leave(::grpc::ClientContext* context, const ::chatroom::Request& request, ::chatroom::Response* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_Leave_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::chatroom::Response>* ChatService::Stub::AsyncLeaveRaw(::grpc::ClientContext* context, const ::chatroom::Request& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::ClientAsyncResponseReader< ::chatroom::Response>::Create(channel_.get(), cq, rpcmethod_Leave_, context, request);
}

::grpc::Status ChatService::Stub::List(::grpc::ClientContext* context, const ::chatroom::ListRequest& request, ::chatroom::ListResponse* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_List_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::chatroom::ListResponse>* ChatService::Stub::AsyncListRaw(::grpc::ClientContext* context, const ::chatroom::ListRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::ClientAsyncResponseReader< ::chatroom::ListResponse>::Create(channel_.get(), cq, rpcmethod_List_, context, request);
}

::grpc::Status ChatService::Stub::Send(::grpc::ClientContext* context, const ::chatroom::SendRequest& request, ::chatroom::Response* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_Send_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::chatroom::Response>* ChatService::Stub::AsyncSendRaw(::grpc::ClientContext* context, const ::chatroom::SendRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::ClientAsyncResponseReader< ::chatroom::Response>::Create(channel_.get(), cq, rpcmethod_Send_, context, request);
}

::grpc::Status ChatService::Stub::SendAll(::grpc::ClientContext* context, const ::chatroom::SendAllRequest& request, ::chatroom::Response* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_SendAll_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::chatroom::Response>* ChatService::Stub::AsyncSendAllRaw(::grpc::ClientContext* context, const ::chatroom::SendAllRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::ClientAsyncResponseReader< ::chatroom::Response>::Create(channel_.get(), cq, rpcmethod_SendAll_, context, request);
}

::grpc::Status ChatService::Stub::RecvAll(::grpc::ClientContext* context, const ::chatroom::RecvAllRequest& request, ::chatroom::ChatResponse* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_RecvAll_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::chatroom::ChatResponse>* ChatService::Stub::AsyncRecvAllRaw(::grpc::ClientContext* context, const ::chatroom::RecvAllRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::ClientAsyncResponseReader< ::chatroom::ChatResponse>::Create(channel_.get(), cq, rpcmethod_RecvAll_, context, request);
}

ChatService::Service::Service() {
  AddMethod(new ::grpc::RpcServiceMethod(
      ChatService_method_names[0],
      ::grpc::RpcMethod::BIDI_STREAMING,
      new ::grpc::BidiStreamingHandler< ChatService::Service, ::chatroom::Message, ::chatroom::Response>(
          std::mem_fn(&ChatService::Service::StartChat), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      ChatService_method_names[1],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< ChatService::Service, ::chatroom::Request, ::chatroom::Response>(
          std::mem_fn(&ChatService::Service::Join), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      ChatService_method_names[2],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< ChatService::Service, ::chatroom::Request, ::chatroom::Response>(
          std::mem_fn(&ChatService::Service::Leave), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      ChatService_method_names[3],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< ChatService::Service, ::chatroom::ListRequest, ::chatroom::ListResponse>(
          std::mem_fn(&ChatService::Service::List), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      ChatService_method_names[4],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< ChatService::Service, ::chatroom::SendRequest, ::chatroom::Response>(
          std::mem_fn(&ChatService::Service::Send), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      ChatService_method_names[5],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< ChatService::Service, ::chatroom::SendAllRequest, ::chatroom::Response>(
          std::mem_fn(&ChatService::Service::SendAll), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      ChatService_method_names[6],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< ChatService::Service, ::chatroom::RecvAllRequest, ::chatroom::ChatResponse>(
          std::mem_fn(&ChatService::Service::RecvAll), this)));
}

ChatService::Service::~Service() {
}

::grpc::Status ChatService::Service::StartChat(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::chatroom::Response, ::chatroom::Message>* stream) {
  (void) context;
  (void) stream;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status ChatService::Service::Join(::grpc::ServerContext* context, const ::chatroom::Request* request, ::chatroom::Response* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status ChatService::Service::Leave(::grpc::ServerContext* context, const ::chatroom::Request* request, ::chatroom::Response* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status ChatService::Service::List(::grpc::ServerContext* context, const ::chatroom::ListRequest* request, ::chatroom::ListResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status ChatService::Service::Send(::grpc::ServerContext* context, const ::chatroom::SendRequest* request, ::chatroom::Response* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status ChatService::Service::SendAll(::grpc::ServerContext* context, const ::chatroom::SendAllRequest* request, ::chatroom::Response* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status ChatService::Service::RecvAll(::grpc::ServerContext* context, const ::chatroom::RecvAllRequest* request, ::chatroom::ChatResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace chatroom

