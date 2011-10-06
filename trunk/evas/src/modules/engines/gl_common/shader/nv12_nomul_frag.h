"#ifdef GL_ES\n"
"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
"precision highp float;\n"
"#else\n"
"precision mediump float;\n"
"#endif\n"
"#endif\n"
"uniform sampler2D tex, texuv;\n"
"varying vec2 tex_c, tex_cuv;\n"
"void main()\n"
"{\n"
"  float y,u,v,vmu,r,g,b;\n"
"  y=texture2D(tex,tex_c).g;\n"
"  u=texture2D(texuv,tex_cuv).g;\n"
"  v=texture2D(texuv,tex_cuv).a;\n"
"\n"
"  u=u-0.5;\n"
"  v=v-0.5;\n"
"  vmu=v*0.813+u*0.391;\n"
"  u=u*2.018;\n"
"  v=v*1.596;\n"
"  y=(y-0.062)*1.164;\n"
"\n"
"  r=y+v;\n"
"  g=y-vmu;\n"
"  b=y+u;\n"
"\n"
"  gl_FragColor=vec4(r,g,b,1.0);\n"
"}\n"
"\n"
"\n"
