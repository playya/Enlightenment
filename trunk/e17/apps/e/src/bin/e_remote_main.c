/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

typedef struct _E_IPC_Opt_Handler E_IPC_Opt_Handler;

struct _E_IPC_Opt_Handler
{
   char *option;
   char *desc;
   int   num_params;
   int   replies;
   int   type;
   int   simple_request_id;
   void (*func) (char **params);
};

/* local subsystem functions */
static int  _e_cb_signal_exit(void *data, int ev_type, void *ev);
static int  _e_ipc_init(void);
static void _e_ipc_shutdown(void);

static int _e_ipc_cb_server_add(void *data, int type, void *event);
static int _e_ipc_cb_server_del(void *data, int type, void *event);
static int _e_ipc_cb_server_data(void *data, int type, void *event);

static void _e_help(void);

/* local subsystem globals */
static Ecore_Ipc_Server *_e_ipc_server  = NULL;
static const char *display_name = NULL;
static int reply_count = 0;
static int reply_expect = 0;

#define SIMPLE_REQ     0
#define SIMPLE_STR_REQ 1
#define FULL_FUNC      2
#define MULTI_STR_REQ  3

#define OREQ(opt, desc, ipc, rep) {opt, desc, 0, rep, SIMPLE_REQ, ipc, NULL}
#define OSTR(opt, desc, ipc, rep) {opt, desc, 1, rep, SIMPLE_STR_REQ, ipc, NULL}
#define OFNC(opt, desc, param, fn, rep) {opt, desc, param, rep, SIMPLE_FUNC, 0, fn}
#define OMUL(opt, desc, ipc, rep, argc) {opt, desc, argc, rep, MULTI_STR_REQ, ipc, NULL}

E_IPC_Opt_Handler handlers[] =
{
   OSTR("-module-load", "Load module OPT1 into memory", E_IPC_OP_MODULE_LOAD, 0),
   OSTR("-module-unload", "Unload (and disable) module OPT1 from memory", E_IPC_OP_MODULE_UNLOAD, 0),
   OSTR("-module-enable", "Enable module OPT1 if not enabled", E_IPC_OP_MODULE_ENABLE, 0),
   OSTR("-module-disable", "Disable module OPT1 if not disabled", E_IPC_OP_MODULE_DISABLE, 0),
   OREQ("-module-list", "List all loaded modules and their states", E_IPC_OP_MODULE_LIST, 1),
   OREQ("-module-dirs-list", "List all modules directories", E_IPC_OP_MODULE_DIRS_LIST, 1),
   OSTR("-bg-set", "Set the background edje file to be OPT1", E_IPC_OP_BG_SET, 0),
   OREQ("-bg-get", "Get the background edje file", E_IPC_OP_BG_GET, 1),
   OREQ("-bg-dirs-list", "Get the background directories", E_IPC_OP_BG_DIRS_LIST, 1),
   OSTR("-font-fallback-remove", "Remove OPT1 from the fontset", E_IPC_OP_FONT_FALLBACK_REMOVE, 0),
   OSTR("-font-fallback-prepend", "Prepend OPT1 to the fontset", E_IPC_OP_FONT_FALLBACK_PREPEND, 0),
   OSTR("-font-fallback-append", "Append OPT1 to the fontset", E_IPC_OP_FONT_FALLBACK_APPEND, 0),
   OREQ("-font-apply", "Apply changes made to the font system", E_IPC_OP_FONT_APPLY, 0),
   OREQ("-font-fallback-list", "List the fallback fonts in order", E_IPC_OP_FONT_FALLBACK_LIST, 1),
   OREQ("-font-available-list", "List available fonts", E_IPC_OP_FONT_AVAILABLE_LIST, 1),
   OREQ("-font-fallback-clear", "Clear font fallback list", E_IPC_OP_FONT_FALLBACK_CLEAR, 0),
   OSTR("-font-default-get", "List the default font associated with OPT1", E_IPC_OP_FONT_DEFAULT_GET, 1),
   OSTR("-font-default-remove", "Remove the default text class OPT1", E_IPC_OP_FONT_DEFAULT_REMOVE, 0),
   OREQ("-font-default-list", "List all configured text classes", E_IPC_OP_FONT_DEFAULT_LIST, 1),
   OMUL("-font-default-set", "Set textclass (OPT1) font (OPT2) and size (OPT3)", E_IPC_OP_FONT_DEFAULT_SET, 0, 3),
   OREQ("-restart", "Restart E17", E_IPC_OP_RESTART, 0),
   OREQ("-shutdown", "Shutdown E17", E_IPC_OP_SHUTDOWN, 0)
};

/* externally accessible functions */
int
main(int argc, char **argv)
{
   int i;
   char *s, buf[1024];
   
   /* fix up DISPLAY to be :N.0 if no .screen is in it */
   s = getenv("DISPLAY");
   if (s)
     {
	char *p;
	
	p = strrchr(s, ':');
	if (!p)
	  {
	     snprintf(buf, sizeof(buf), "DISPLAY=%s:0.0", s);
	     putenv(strdup(buf));
	  }
	else
	  {
	     p = strrchr(p, '.');
	     if (!p)
	       {
		  snprintf(buf, sizeof(buf), "DISPLAY=%s.0", s);
		  putenv(strdup(buf));
	       }
	  }
     }
   
   /* handle some command-line parameters */
   display_name = (const char *)getenv("DISPLAY");
   for (i = 1; i < argc; i++)
     {
	if ((!strcmp(argv[i], "-display")) && (i < (argc - 1)))
	  {
	     i++;
	     display_name = argv[i];
	  }
	else if ((!strcmp(argv[i], "-h")) ||
		 (!strcmp(argv[i], "-help")) ||
		 (!strcmp(argv[i], "--h")) ||
		 (!strcmp(argv[i], "--help")))
	  {
	     _e_help();
	     exit(0);
	  }
     }
  
   /* basic ecore init */
   if (!ecore_init())
     {
	printf("ERROR: Enlightenment_remote cannot Initialize Ecore!\n"
	       "Perhaps you are out of memory?\n");
	exit(-1);
     }
   ecore_app_args_set((int)argc, (const char **)argv);
   /* setup a handler for when e is asked to exit via a system signal */
   if (!ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, _e_cb_signal_exit, NULL))
     {
	printf("ERROR: Enlightenment_remote cannot set up an exit signal handler.\n"
	       "Perhaps you are out of memory?\n");
	exit(-1);
     }
   /* init ipc */
   if (!ecore_ipc_init())
     {
	printf("ERROR: Enlightenment_remote cannot initialize the ipc system.\n"
	       "Perhaps you are out of memory?\n");
	exit(-1);
     }
   
   /* setup e ipc service */
   if (!_e_ipc_init())
     {
	printf("ERROR: Enlightenment_remote cannot set up the IPC socket.\n"
	       "Maybe try the '-display :0.0' option?\n");
	exit(-1);
     }

   /* start our main loop */
   ecore_main_loop_begin();
   
   _e_ipc_shutdown();
   ecore_ipc_shutdown();
   ecore_shutdown();
   
   /* just return 0 to keep the compiler quiet */
   return 0;
}

/* local subsystem functions */
static int
_e_cb_signal_exit(void *data, int ev_type, void *ev)
{
   /* called on ctrl-c, kill (pid) (also SIGINT, SIGTERM and SIGQIT) */
   ecore_main_loop_quit();
   return 1;
}

static int
_e_ipc_init(void)
{
   char buf[1024];
   char *disp;
   
   disp = (char *)display_name;
   if (!disp) disp = ":0";
   snprintf(buf, sizeof(buf), "enlightenment-(%s)", disp);
   _e_ipc_server = ecore_ipc_server_connect(ECORE_IPC_LOCAL_USER, buf, 0, NULL);
   /* FIXME: we shoudl also try the generic ":0" if the display is ":0.0" */
   /* similar... */
   if (!_e_ipc_server) return 0;
   
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_ADD, _e_ipc_cb_server_add, NULL);
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DEL, _e_ipc_cb_server_del, NULL);
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DATA, _e_ipc_cb_server_data, NULL);
   
   return 1;
}

static void
_e_ipc_shutdown(void)
{
   if (_e_ipc_server)
     {
	ecore_ipc_server_del(_e_ipc_server);
	_e_ipc_server = NULL;
     }
}

static int
_e_ipc_cb_server_add(void *data, int type, void *event)
{
   Ecore_Ipc_Event_Server_Add *e;
   int argc;
   char **argv;
   int i;
   int process_count = 0;
   
   e = event;
   ecore_app_args_get(&argc, &argv);
   for (i = 1; i < argc; i++)
     {
	char *v, *p;
	int j;
	int k;
	int data_size;
	
	for (j = 0; j < (int)(sizeof(handlers) / sizeof(E_IPC_Opt_Handler)); j++)
	  {
	     E_IPC_Opt_Handler *handler;
	     
	     handler = &handlers[j];
	     if (!strcmp(handler->option, argv[i]))
	       {
		  if (i >= (argc - handler->num_params))
		    {
		       printf("ERROR: option %s expects %i parameters\n",
			      handler->option, handler->num_params);
		       exit(-1);
		    }
		  else
		    {
		       switch (handler->type)
			 {
			  case SIMPLE_REQ:
			    ecore_ipc_server_send(_e_ipc_server,
						  E_IPC_DOMAIN_REQUEST,
						  handler->simple_request_id,
						  0/*ref*/, 0/*ref_to*/, 0/*response*/,
						  NULL, 0);
			    break;
			  case SIMPLE_STR_REQ:
			    v = argv[i + 1];
			    ecore_ipc_server_send(_e_ipc_server,
						  E_IPC_DOMAIN_REQUEST,
						  handler->simple_request_id,
						  0/*ref*/, 0/*ref_to*/, 0/*response*/,
						  v, strlen(v));
			    break;
			  case MULTI_STR_REQ:
			    /* pack up the data "<str>0<str>0" */
			    data_size = 0;
			    for(k = 0; k < handler->num_params; k++) {
				data_size += strlen(argv[ i + 1 + k ]);
				data_size++; /* NULL Pad */
			    }			     
			    v = malloc(data_size);
			    p = v;	    			
			    for(k = 0; k < handler->num_params; k++) {
				 strcpy(p, argv[ i + 1 + k]);
				 p += strlen(argv[ i + 1 + k]);
				 *p = 0;
				 p++;
			    }	
			    ecore_ipc_server_send(_e_ipc_server,
						  E_IPC_DOMAIN_REQUEST,
						  handler->simple_request_id,
						  0/*ref*/, 0/*ref_to*/, 0/*response*/,
						  v, data_size);
			    free(v);
 
			    break;
			  case FULL_FUNC:
			    handler->func(argv + i + 1);
			    break;
			  default:
			    break;
			 }
		       process_count++;
		       reply_expect += handler->replies;
		       i += handler->num_params;
		       break;
		    }
	       }
	  }
     }
   if (process_count <= 0)
     _e_help();
   if (reply_count >= reply_expect) ecore_main_loop_quit();
   return 1;
}

static int
_e_ipc_cb_server_del(void *data, int type, void *event)
{
   Ecore_Ipc_Event_Server_Del *e;
   
   e = event;
   return 1; 
}

static int
_e_ipc_cb_server_data(void *data, int type, void *event)
{
   Ecore_Ipc_Event_Server_Data *e;
   
   e = event;
   /* FIXME: should make this function/callback based in a table like the */
   /* option handlers... */
   printf("REPLY: BEGIN\n");
   switch (e->minor)
     {  
      case E_IPC_OP_MODULE_LIST_REPLY:
	if (e->data)
	  {
	     char *p;
	     
	     p = e->data;
	     while (p < (char *)(e->data + e->size))
	       {
		  char *name;
		  char  enabled;
		  
		  name = p;
		  p += strlen(name);
		  if (p < (char *)(e->data + e->size))
		    {
		       p++;
		       if (p < (char *)(e->data + e->size))
			 {
			    enabled = *p;
			    p++;
			    printf("REPLY: MODULE NAME=\"%s\" ENABLED=%i\n",
				   name, (int)enabled);
			 }
		    }
	       }
	  }
	else
	  printf("REPLY: MODULE NONE\n");
	break;
	case E_IPC_OP_MODULE_DIRS_LIST_REPLY:
	if (e->data)
	  {
	     char *p;

	     p = e->data;
	     while (p < (char *)(e->data + e->size))
	       {
		  char *dir;

		  dir = p;
		  printf("REPLY: MODULE DIR=%s\n", dir);
		  p += strlen(dir) + 1;
	       }
	  }
      break;
      case E_IPC_OP_BG_GET_REPLY:
      if (e->data)
	{
	   printf("REPLY: %s\n", e->data);
	}
      break;
      case E_IPC_OP_BG_DIRS_LIST_REPLY:
      if (e->data)
	{
	   char *p;
 
	   p = e->data;
	   while (p < (char *)(e->data + e->size))
	     {
	         char *dir;

		 dir = p;
		 printf("REPLY: BG DIR=%s\n", dir);
		 p += strlen(dir) + 1;
	     }
	 }
      break;
      case E_IPC_OP_FONT_FALLBACK_LIST_REPLY:
	if (e->data)
	  {
	     char *p;
	     
	     p = e->data;
	     while (p < (char *)(e->data + e->size))
	       {
		  char *name;
		  
		  name = p;
		  p += strlen(name);
		  if (p < (char *)(e->data + e->size))
		    {
			printf("REPLY: FALLBACK NAME=\"%s\"\n", name);
		    }
		  p++;
	       }
	  }
	else
	  printf("REPLY: FALLBACK NONE\n");
	break;
      case E_IPC_OP_FONT_AVAILABLE_LIST_REPLY:
        if (e->data)
          {
             char *p;
          
             p = e->data;
             while (p < (char *)(e->data + e->size))
              	{
              	  char *name;
              	  name = p;
 		  p += strlen(name);
		  if (p < (char *)(e->data + e->size))
		    {
			printf("REPLY: AVAILABLE NAME=\"%s\"\n", name);
		    }
		  p++;             	    
              	}
          }
        else
          printf("REPLY: AVAILABLE NONE\n"); 
        break;   
      case E_IPC_OP_FONT_DEFAULT_GET_REPLY:
        if (e->data)
          {
             char *text_class, *name;
             char *p;
	     char size;
	     
	     p = e->data;
		  
	     text_class = p;
	     p += strlen(text_class) + 1;
	     if (p < (char *)(e->data + e->size))
	       {
		       name = p;
		       p  += strlen(name) + 1;     
		       if (p < (char *)(e->data + e->size))
			 {
			   	 size = *p;
			    	 printf("REPLY: DEFAULT TEXT_CLASS=\"%s\" NAME=\"%s\" SIZE=%i\n",
				   text_class, name, (int)size);
				 p++;
			  
			 }
	       }
          }
        else
          printf("REPLY: DEFAULT NONE\n"); 
        break;
      case E_IPC_OP_FONT_DEFAULT_LIST_REPLY:
        if (e->data)
          {
             char *text_class, *name;
             char *p;
	     char size;
	     
	     p = e->data;
		
	while (p < (char *)(e->data + e->size))
	{  
	     text_class = p;
	     p += strlen(text_class) + 1;
	     if (p < (char *)(e->data + e->size))
	       {
		       name = p;
		       p += strlen(name) + 1;
		       if (p < (char *)(e->data + e->size))
			 {
			   	 size = *p;
			    	 printf("REPLY: DEFAULT TEXT_CLASS=\"%s\" NAME=\"%s\" SIZE=%i\n",
				   text_class, name, (int)size);
				 p++;
			 }
	       }
	  }
          }
        else
          printf("REPLY: DEFAULT NONE\n"); 
        break;	
      default:
	break;
     }
   printf("REPLY: END\n");
   reply_count++;
   if (reply_count >= reply_expect) ecore_main_loop_quit();
   return 1;
}

static void
_e_help(void)
{
   int j, k, l;
   E_IPC_Opt_Handler *handler;
   char buf[128];
   int parsize = 0, opsize = 0;
   
   printf("OPTIONS:\n");
   for (j = 0; j < (int)(sizeof(handlers) / sizeof(E_IPC_Opt_Handler)); j++)
     {
	handler = &handlers[j];
	if ((int)strlen(handler->option) > parsize) parsize = strlen(handler->option);
	l = 0;
	for (k = 0; k < handler->num_params; k++)
	  {
	     snprintf(buf, sizeof(buf), " OPT%i", k + 1);
	     l += strlen(buf);
	  }
	if (l > opsize) opsize = l;
     }
   for (j = 0; j < (int)(sizeof(handlers) / sizeof(E_IPC_Opt_Handler)); j++)
     {
	handler = &handlers[j];
	printf("  %s", handler->option);
	l = parsize - strlen(handler->option);
	for (k = 0; k < l; k++) printf(" ");
	l = 0;
	for (k = 0; k < handler->num_params; k++)
	  {
	     snprintf(buf, sizeof(buf), " OPT%i", k + 1);
	     printf("%s", buf);
	     l += strlen(buf);
	  }
	while (l < opsize)
	  {
	     printf(" ");
	     l++;
	  }
	printf("  - %s\n", handler->desc);
     }
}
