"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"uniform sampler2D tex;\n"
"varying vec4 col;\n"
"void main()\n"
"{\n"
"   gl_FragColor = col;\n"
"}\n"
