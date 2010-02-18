"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"attribute vec4 vertex;\n"
"attribute vec4 color;\n"
"attribute vec2 tex_coord;\n"
"uniform mat4 mvp;\n"
"varying vec4 col;\n"
"varying vec2 tex_c;\n"
"void main()\n"
"{\n"
"   gl_Position = mvp * vertex;\n"
"   col = color;\n"
"   tex_c = tex_coord;\n"
"}\n"
