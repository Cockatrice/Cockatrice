New-Item -ItemType directory -Path $env:APPVEYOR_BUILD_FOLDER\build
Set-Location -Path $env:APPVEYOR_BUILD_FOLDER\build
$zlibdir = "c:\zlib-release"
$openssldir = "C:\openssl-release"
$protodir = "c:\protobuf-release"
$protoc = "c:\protobuf-release\bin\protoc.exe"
$mysqldll = "c:\Program Files\MySQL\MySQL Server 5.7\lib\libmysql.dll"
cmake .. -G "$env:cmake_generator" -T "$env:cmake_toolset" "-DCMAKE_PREFIX_PATH=c:/Qt/$env:qt_ver;$protodir;$zlibdir;$openssldir" "-DWITH_SERVER=1" "-DPROTOBUF_PROTOC_EXECUTABLE=$protoc" "-DMYSQLCLIENT_LIBRARIES=$mysqldll"
