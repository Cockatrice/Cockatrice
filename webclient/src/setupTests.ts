import protobuf from 'protobufjs';

class MockProtobufRoot {
  load() {}
}

(protobuf as any).Root = MockProtobufRoot;
