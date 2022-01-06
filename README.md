# tiny inference engine
Welcome to **tiny inference engine**!  
This repository contains a small Client/Server system to perform Machine Learning inference on high load systems.
This system allows to scale CPU & GPU resources serving multiple clients using a single server instance: it is mainly suitable for distributed scenarios and demanding applications.

## Server
The server application is written in modern, cross-platform C++ and can run on the 3 major platforms: Windows and Linux with both GPU and CPU support, while on MacOS only CPU is supported.
It is also possible to run the server in a Docker container.

## Client
The client is available as a library in order to be consumed by any application.
It is written in C++ and can run on Windows, Linux and MacOS.
See examples for more details.

### Communication Protocol: gRPC & HTTP
The inter process communication layer uses gRPC since it provides better performances over plain HTTP.
Because of this it is possible to make inference calls using only the client library.
The server however exposes also some HTTP endpoints to provide metrics and allow dynamic tuning at runtime so that other applications (e.g. curl) can be used to interact with it.

### Machine Learning Backend: ONNX Runtime
Currently only ONNX Runtime is supported as server backend.
The client is completely decoupled from the server backend: client applications must apply preprocessing in order to send properly formatted requests for the model required on server side.
