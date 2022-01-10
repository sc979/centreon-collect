#!/bin/bash

cd resources
python3 -m grpc_tools.protoc -I../../centreon-engine/enginerpc --python_out=. --grpc_python_out=. engine.proto
python3 -m grpc_tools.protoc -I../../centreon-broker/core/src --python_out=. --grpc_python_out=. broker.proto
cd ..
