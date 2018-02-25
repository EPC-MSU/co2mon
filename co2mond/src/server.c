#include "include/platform.h"
#include "include/ringbuf.h"
#include "include/server.h"

#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	char *head;
	char *body_start;
	char *js_payload;
	char *body_end;
} html_response;

typedef struct {
	int value;
	double temp;
	time_t time;
} data_structure;

static html_response resp;
struct MHD_Daemon *d;
ringbuf_t rbuf = NULL;

static int
ahc_echo (void *cls,
	struct MHD_Connection *connection,
	const char *url,
	const char *method,
	const char *version,
	const char *upload_data, size_t *upload_data_size, void **ptr)
{
	static int aptr;
	html_response *resp = cls;
	
	size_t bytestocopy = ringbuf_bytes_used(rbuf);
	if( bytestocopy % sizeof(data_structure) )
	{
		fprintf(stderr, "Error: data disalignment\n");
		return MHD_NO;
	}
	data_structure *datacopy = malloc(bytestocopy);
	ringbuf_memcpy_peek(datacopy, rbuf, bytestocopy);
	*(resp->js_payload) = 0;
	char strbuf[40];
	// Trace 1
	strcat(resp->js_payload, "\nvar trace1 = {\nx: ['");
	for ( int i = 0; i < bytestocopy / sizeof(data_structure); i++ )
	{
		strftime (strbuf, 40, "%F %T", localtime(&datacopy[i].time) );
		strcat(resp->js_payload, strbuf);
		if(i != bytestocopy / sizeof(data_structure) - 1) // Except last time
		{
			strcat(resp->js_payload, "', '");
		}
	}
	strcat(resp->js_payload, "'],\ny: [");
	for ( int i = 0; i < bytestocopy / sizeof(data_structure); i++ )
	{
		sprintf(strbuf, "%i", datacopy[i].value);
		strcat(resp->js_payload, strbuf);
		if(i != bytestocopy / sizeof(data_structure) - 1) // Except last time
		{
			strcat(resp->js_payload, ", ");
		}
	}
	strcat(resp->js_payload, "],\nname: 'CO2 ppm',\ntype: 'scatter'\n};\n");
	// Trace 2
	strcat(resp->js_payload, "var trace2 = {\nx: ['");
	for ( int i = 0; i < bytestocopy / sizeof(data_structure); i++ )
	{
		strftime (strbuf, 40, "%F %T", localtime(&datacopy[i].time) );
		strcat(resp->js_payload, strbuf);
		if(i != bytestocopy / sizeof(data_structure) - 1) // Except last time
		{
			strcat(resp->js_payload, "', '");
		}
	}
	strcat(resp->js_payload, "'],\ny: [");
	for ( int i = 0; i < bytestocopy / sizeof(data_structure); i++ )
	{
		sprintf(strbuf, "%f", datacopy[i].temp);
		strcat(resp->js_payload, strbuf);
		if(i != bytestocopy / sizeof(data_structure) - 1) // Except last time
		{
			strcat(resp->js_payload, ", ");
		}
	}
	strcat(resp->js_payload, "],\nname: 'Temperature',\nyaxis: 'y2',\ntype: 'scatter'\n};\n");
	
	strcat(resp->js_payload, "var data = [trace1, trace2]; var layout = {title: 'CO2 monitor',yaxis: {title: 'CO2 ppm'},yaxis2: {title: 'Temperature',titlefont: {color: 'rgb(148, 103, 189)'},tickfont: {color: 'rgb(148, 103, 189)'},overlaying: 'y',side: 'right'}}; Plotly.newPlot('myDiv', data, layout);");
	
	int bufsize = strlen(resp->head) + strlen(resp->body_start) + strlen(resp->js_payload) + strlen(resp->body_end);
	char *me = malloc(bufsize+1); // memory must be freed by MHD by use of MHD_RESPMEM_MUST_FREE
	strcpy(me, resp->head);
	strcat(me, resp->body_start);
	strcat(me, resp->js_payload);
	strcat(me, resp->body_end);
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
  response = MHD_create_response_from_buffer (strlen (me),
					      (void *) me,
					      MHD_RESPMEM_MUST_FREE);
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
	resp.head = "<head><!-- Plotly.js --><script src='https://cdn.plot.ly/plotly-latest.min.js'></script></head>";
	resp.body_start = "<body><!-- Plotly chart will be drawn inside this DIV --><div id='myDiv' style='width:100%;height:100%'></div><script>";
	resp.body_end = "</script></body>";
	resp.js_payload = malloc(bufcount * 80 + 1024); // Predicted maximum payload string size with static and dynamic part
	
	rbuf = ringbuf_new(sizeof(data_structure) * bufcount);

	d = MHD_start_daemon (// MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG | MHD_USE_POLL,
			MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
                        port,
                        NULL, NULL, &ahc_echo, &resp,
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
	if(resp.js_payload)
	{
		free(resp.js_payload);
	}
	if(rbuf)
	{
		ringbuf_free(&rbuf);
	}
	return 0;
}
