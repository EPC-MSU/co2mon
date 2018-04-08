#include "../../ringbuf/include/ringbuf.h"
#include "../include/server.h"

#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
	int value;
	double temp;
	time_t time;
} data_structure;

struct MHD_Daemon *d;
ringbuf_t rbuf = NULL;

static char *response = NULL;
static size_t growoffset;
static void grow_init(char* dst)
{
	*dst = 0;
	growoffset = 0;
}
static void grow(char* dst, const char* src)
{
	strcat(dst + growoffset, src);
	growoffset += strlen(src);
}

static int
ahc_echo (void *cls,
	struct MHD_Connection *connection,
	const char *url,
	const char *method,
	const char *version,
	const char *upload_data, size_t *upload_data_size, void **ptr)
{
	static int aptr;
	char *resp = cls;
	
	size_t bytestocopy = ringbuf_bytes_used(rbuf);
	if( bytestocopy % sizeof(data_structure) )
	{
		fprintf(stderr, "Error: data disalignment\n");
		return MHD_NO;
	}
	data_structure *datacopy = malloc(bytestocopy);
	ringbuf_memcpy_peek(datacopy, rbuf, bytestocopy);
	grow_init(resp);
	
	grow(resp, "<head><!-- Plotly.js --><script src='https://cdn.plot.ly/plotly-latest.min.js'></script></head>");
	grow(resp, "<body><!-- Plotly chart will be drawn inside this DIV --><div id='myDiv' style='width:100%;height:100%'></div><script>");
	
	char strbuf[40];
	// Trace 1
	grow(resp, "\nvar trace1 = {\nx: ['");
	for ( int i = 0; i < bytestocopy / sizeof(data_structure); i++ )
	{
		strftime (strbuf, 40, "%F %T", localtime(&datacopy[i].time) );
		grow(resp, strbuf);
		if(i != bytestocopy / sizeof(data_structure) - 1) // Except last time
		{
			grow(resp, "', '");
		}
	}
	grow(resp, "'],\ny: [");
	for ( int i = 0; i < bytestocopy / sizeof(data_structure); i++ )
	{
		sprintf(strbuf, "%i", datacopy[i].value);
		grow(resp, strbuf);
		if(i != bytestocopy / sizeof(data_structure) - 1) // Except last time
		{
			grow(resp, ", ");
		}
	}
	grow(resp, "],\nname: 'CO2 ppm',\ntype: 'scatter'\n};\n");
	// Trace 2
	grow(resp, "var trace2 = {\nx: ['");
	for ( int i = 0; i < bytestocopy / sizeof(data_structure); i++ )
	{
		strftime (strbuf, 40, "%F %T", localtime(&datacopy[i].time) );
		grow(resp, strbuf);
		if(i != bytestocopy / sizeof(data_structure) - 1) // Except last time
		{
			grow(resp, "', '");
		}
	}
	grow(resp, "'],\ny: [");
	for ( int i = 0; i < bytestocopy / sizeof(data_structure); i++ )
	{
		sprintf(strbuf, "%f", datacopy[i].temp);
		grow(resp, strbuf);
		if(i != bytestocopy / sizeof(data_structure) - 1) // Except last time
		{
			grow(resp, ", ");
		}
	}
	grow(resp, "],\nname: 'Temperature',\nyaxis: 'y2',\ntype: 'scatter'\n};\n");
	
	grow(resp, "var data = [trace1, trace2]; var layout = {title: 'CO2 monitor',yaxis: {title: 'CO2 ppm'},yaxis2: {title: 'Temperature',titlefont: {color: 'rgb(148, 103, 189)'},tickfont: {color: 'rgb(148, 103, 189)'},overlaying: 'y',side: 'right'}}; Plotly.newPlot('myDiv', data, layout);");
	
	grow(resp, "</script></body>");
	
	free(datacopy);
	
	struct MHD_Response *response;
	int ret;

  if (0 != strcmp (method, "GET"))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *ptr)
    {
      /* do never respond on first call */
      *ptr = &aptr;
      return MHD_YES;
    }
  *ptr = NULL;                  /* reset when done */
  response = MHD_create_response_from_buffer (strlen (resp),
					      (void *) resp,
					      MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;
}

int server_feed (int nval, double temp)
{
	data_structure ds;
	ds.value = nval;
	ds.temp = temp;
	ds.time = time(NULL);
	if(ringbuf_bytes_free(rbuf) < sizeof(data_structure))
	{
		ringbuf_memcpy_from(NULL, rbuf, sizeof(data_structure));
	}
	ringbuf_memcpy_into(rbuf, &ds, sizeof(data_structure));
	return 0;
}

int server_init(long int port, int bufcount)
{
	response = malloc(bufcount * 80 + 1024); // Predicted maximum response string size with static and dynamic part
	
	rbuf = ringbuf_new(sizeof(data_structure) * bufcount);

	d = MHD_start_daemon (// MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG | MHD_USE_POLL,
			MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
                        port,
                        NULL, NULL, &ahc_echo, response,
			MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
			MHD_OPTION_END);
	if (d == NULL)
		return 1;
	return 0;
}

int server_exit(void)
{
	if(d)
	{
		MHD_stop_daemon (d);
	}
	if(response)
	{
		free(response);
	}
	if(rbuf)
	{
		ringbuf_free(&rbuf);
	}
	return 0;
}
