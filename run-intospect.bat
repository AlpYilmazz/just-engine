run ^
    "justengine/bin/introspect.exe" ^
    curl-test.c ^
    introspect_gen__curl_test.h ^
    -Ijustengine/include/openssl -Ijustengine/include/curl -Ijustengine/include/raylib -Ijustengine/include/clay -Ijustengine/include