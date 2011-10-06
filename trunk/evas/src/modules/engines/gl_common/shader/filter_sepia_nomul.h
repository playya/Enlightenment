"#ifdef GL_ES\n"
"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
"precision highp float;\n"
"#else\n"
"precision mediump float;\n"
"#endif\n"
"#endif\n"
"uniform sampler2D tex;\n"
"varying vec4 col;\n"
"varying vec2 tex_c;\n"
"void main()\n"
"{\n"
"	vec3 inp = texture2D(tex,tex_c.xy).abg;\n"
"	gl_FragColor.r = dot(inp, vec3(.393, .769, .189));\n"
"	gl_FragColor.g = dot(inp, vec3(.349, .686, .168));\n"
"	gl_FragColor.b = dot(inp, vec3(.272, .534, .131));\n"
"	gl_FragColor.a = texture2D(tex,tex_c.xy).r;\n"
"}\n"
