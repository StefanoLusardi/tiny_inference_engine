file(MAKE_DIRECTORY ${PROTO_OUT_DIR})
set(proto_include "${PROTO_OUT_DIR}/${PROTO_FILENAME}.pb.h")
set(grpc_include "${PROTO_OUT_DIR}/${PROTO_FILENAME}.grpc.pb.h")
set(proto_src "${PROTO_OUT_DIR}/${PROTO_FILENAME}.pb.cc")
set(grpc_src "${PROTO_OUT_DIR}/${PROTO_FILENAME}.grpc.pb.cc")

add_custom_command(
    OUTPUT ${proto_src} ${proto_include} ${grpc_src} ${grpc_include}
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --grpc_out=${PROTO_OUT_DIR} --cpp_out=${PROTO_OUT_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_PROGRAM} ${PROTO_FILE}
    WORKING_DIRECTORY ${PROTO_IN_DIR}
    DEPENDS ${PROTO_IN_DIR}/${PROTO_FILE}
    COMMENT "Generating gRPC files"
    VERBATIM
)