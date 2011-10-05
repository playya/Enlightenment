"#ifdef GL_ES\n"
"precision highp float;\n"
"#endif\n"
"attribute vec4 vertex;\n"
"attribute vec4 color;\n"
"attribute vec2 tex_coord, tex_coord2;\n"
"uniform mat4 mvp;\n"
"varying vec4 col;\n"
"varying vec2 tex_c, tex_cuv;\n"
"void main()\n"
"{\n"
"   gl_Position = mvp * vertex;\n"
"   col = color;\n"
"   tex_c = tex_coord;\n"
"   tex_cuv = vec2(tex_coord2.x * 0.5, tex_coord2.y);\n"
"}\n"
