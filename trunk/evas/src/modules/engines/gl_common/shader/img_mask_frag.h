"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"uniform sampler2D tex, texm;\n"
"varying vec4 col;\n"
"varying vec2 tex_c, tex_cm;\n"
"void main()\n"
"{\n"
"   gl_FragColor = texture2D(texm, tex_cm.xy).aaaa * texture2D(tex, tex_c.xy).rgba * col;\n"
"}\n"
"\n"
