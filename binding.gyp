{
  "targets": [
    {
      "target_name": "djondb",
      "sources": [ "nodeutil.cpp", "djondb.cpp", "wrapconnectionmanager.cpp", "wrapconnection.cpp", "wrapcursor.cpp" ],
      "cflags!": [ "-fno-exceptions"],
      "cflags_cc!": [ "-fno-exceptions"],
		"include_dirs": ["includes"],
		'link_settings': {
			'libraries': [
				'-ldjon-client'
				]
		}
	 }
  ]
}
