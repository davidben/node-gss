{
    "targets": [
        {
            "target_name": "gss",
            "sources": [
                "src/context.cc",
                "src/cred.cc",
                "src/gss.cc",
                "src/message.cc",
                "src/misc.cc",
                "src/name.cc",
                "src/util.cc",
            ],
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
