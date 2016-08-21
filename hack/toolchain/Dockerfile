FROM golang:1.7
RUN apt-get -y update && \
    apt-get -y install curl git tar zip xz-utils && \
    apt-get -y clean && \
    curl -sSL https://github.com/google/protobuf/releases/download/v3.0.0/protoc-3.0.0-linux-x86_64.zip >/tmp/protoc.zip && \
    cd /usr/local && unzip /tmp/protoc.zip && rm /tmp/protoc.zip && \
    chmod a+rx /usr/local/bin/protoc && \
    curl -sSL https://nodejs.org/dist/v4.5.0/node-v4.5.0-linux-x64.tar.xz | tar -C /usr/local -Jx --strip-components=1 && \
    go get -v github.com/golang/protobuf/protoc-gen-go && \
    go get -v github.com/alecthomas/gometalinter && \
    go get -v golang.org/x/tools/cmd/... && \
    go get -v github.com/FiloSottile/gvt && \
    go get -v github.com/smartystreets/goconvey && \
    go get -v github.com/robotalks/tbus/codegen/cmd/... && \
    cp -rf /go/src/github.com/robotalks/tbus/proto/tbus /usr/local/include/ && \
    gometalinter --install && \
    chmod -R a+rx /usr/local/include && \    
    chmod -R a+rw /go
