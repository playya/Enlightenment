"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"attribute vec4 vertex;\n"
"attribute vec4 color;\n"
"attribute vec2 tex_coord;\n"
"attribute float r;\n"
"uniform mat4 mvp;\n"
"uniform sampler1D tex_blur;\n"
"varying float weight;\n"
"varying vec4 col;\n"
"varying vec2 tex_c;\n"
"\n"
"void main(){\n"
"	/* FIXME: This index shoudl be tweaked so for \n"
"		radius 1, I want 3 points at 1/4, 2/4, 3/4 */\n"
"	/*\n"
"	for (float i = 0 ; i <= radius * 2 ; i ++){\n"
"		float pos = i;\n"
"		float r = float(radius);\n"
"		weight += sampler1D(tex_blur, 1.0 / (r * 2.0 * pos));\n"
"	}*/\n"
"	for (float i = 0.0 ; i < r * 2.0 ; i += 1.0){\n"
"		weight += sampler1D(tex_blur, 1.0 / (r * 2.0 * i));\n"
"	}\n"
"	gl_Position = mvp * vertex;\n"
"	col = color;\n"
"	tex_c = tex_coord;\n"
"}\n"
