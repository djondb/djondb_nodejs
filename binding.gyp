{
  "targets": [
    {
      "target_name": "djondb",
      "sources": [ "nodeutil.cpp", "djondb.cpp", "wrapconnectionmanager.cpp", "wrapconnection.cpp", "wrapcursor.cpp" ],
      "cflags!": [ "-fno_exceptions"],
      "cflags_cc!": [ "-fno_exceptions"],
		"include_dirs": ["includes"],
		'link_settings': {
			'libraries': [
				'-ldjon-client'
				]
		},
		 'conditions': [
			['OS=="mac"', {
			  'xcode_settings': {
				 'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
          		'GCC_ENABLE_CPP_RTTI': 'YES',              # -fno-rtti
				 }
				}
         ]]
	 }
  ]
}
