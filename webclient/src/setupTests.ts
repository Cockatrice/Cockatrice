import protobuf from 'protobufjs';

// ensure jest-dom is always available during testing to cut down on boilerplate
import '@testing-library/jest-dom';

class MockProtobufRoot {
  load() {}
}

(protobuf as any).Root = MockProtobufRoot;
