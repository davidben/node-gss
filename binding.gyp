{
    "targets": [
        {
            "target_name": "gss",
            "sources": [ "src/gss.cc" ],
            "link_settings": {
                "libraries": [
                    "-lgssapi_krb5"
                ]
            },
            "cflags": [
                "-W", "-Wall", "-Wno-unused-parameter"
            ]
        }
    ]
}
