if (Test-Path c:\openssl-release) {
    echo "Using 'OpenSSL' from cache!"
} else {
    Invoke-WebRequest "https://indy.fulgan.com/SSL/openssl-$env:openssl_ver.zip" -OutFile c:\openssl-$env:openssl_ver.zip
    Expand-Archive -Path c:\openssl-$env:openssl_ver.zip -DestinationPath c:\openssl-release
    Set-Location -Path C:\openssl-release
}

if (Test-Path c:\protobuf-release) {
    echo "Using 'protobuf' from cache!"
} else {
    Invoke-WebRequest "https://github.com/google/protobuf/releases/download/v$env:protobuf_ver/protobuf-cpp-$env:protobuf_ver.zip" -OutFile c:\protobuf-cpp-$env:protobuf_ver.zip
    Expand-Archive -Path c:\protobuf-cpp-$env:protobuf_ver.zip -DestinationPath c:\
    Set-Location -Path C:\protobuf-$env:protobuf_ver\cmake
    cmake . -G "$env:cmake_generator" -T "$env:cmake_toolset" -Dprotobuf_BUILD_TESTS=0 -Dprotobuf_MSVC_STATIC_RUNTIME=0 -DCMAKE_INSTALL_PREFIX=c:/protobuf-release
    msbuild INSTALL.vcxproj /p:Configuration=Release
}

if (Test-Path c:\zlib-release) {
    echo "Using 'zlib' from cache!"
} else {
    Invoke-WebRequest "https://github.com/madler/zlib/archive/v$env:zlib_ver.zip" -OutFile c:\zlib-$env:zlib_ver.zip
    Expand-Archive -Path c:\zlib-$env:zlib_ver.zip -DestinationPath c:\
    Set-Location -Path C:\zlib-$env:zlib_ver
    cmake . -G "$env:cmake_generator" -T "$env:cmake_toolset" -DCMAKE_INSTALL_PREFIX=c:/zlib-release
    msbuild INSTALL.vcxproj /p:Configuration=Release
}
