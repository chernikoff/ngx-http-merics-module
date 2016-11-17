/**
 *   \file ngx_http_metrics_module.c
 *   \brief Nginx metrics module 
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
  ngx_uint_t enable;
} ngx_http_metrics_main_conf_t;

static ngx_int_t
ngx_http_metrics_init(ngx_conf_t *cf);

static void *
ngx_http_metrics_create_main_conf(ngx_conf_t *cf);

static char *
ngx_http_metrics_enable(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *
ngx_http_metrics_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

ngx_int_t
ngx_http_metrics_status_handler(ngx_http_request_t *r);
ngx_int_t
ngx_http_metrics_handler(ngx_http_request_t *r);

static ngx_command_t
ngx_http_metrics_commands[] = {

  { ngx_string("metrics_enable"),
    NGX_HTTP_SRV_CONF | NGX_CONF_NOARGS,
    ngx_http_metrics_enable,
    NGX_HTTP_MAIN_CONF_OFFSET,
    0,
    NULL },

  { ngx_string("metrics_status"),
    NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
    ngx_http_metrics_status,
    0,
    0,
    NULL },
  
  ngx_null_command
};

static ngx_http_module_t
ngx_http_metrics_module_ctx = {
  NULL,                                   /* preconfiguration */
  ngx_http_metrics_init,                  /* postconfiguration */
  
  ngx_http_metrics_create_main_conf,      /* create main configuration */
  NULL,                                   /* init main configuration */

  NULL,                                   /* create server configuration */
  NULL,                                   /* merge server configuration */

  NULL,                                   /* create location configuration */
  NULL                                    /* merge location configuration */
};

ngx_module_t
ngx_http_metrics_module = {
  NGX_MODULE_V1,
  &ngx_http_metrics_module_ctx,           /* module context */
  ngx_http_metrics_commands,              /* module commands */
  NGX_HTTP_MODULE,                        /* module type */
  NULL,                                   /* init master */
  NULL,                                   /* init module */
  NULL,                                   /* init process */
  NULL,                                   /* init thread */
  NULL,                                   /* exit thread */
  NULL,                                   /* exit process */
  NULL,                                   /* exit master */
  NGX_MODULE_V1_PADDING
};
  
static ngx_int_t
ngx_http_metrics_init(ngx_conf_t *cf)
{
  ngx_http_handler_pt       *h;
  ngx_http_core_main_conf_t *cmcf;

  fprintf(stderr, "METRICS: call ngx_http_metrics_init\n");

  cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

  h = ngx_array_push(&cmcf->phases[NGX_HTTP_LOG_PHASE].handlers);

  if (h == NULL) {
    fprintf(stderr, "METRICS: Can't create end handler\n");
    return NGX_ERROR;
  }

  *h = ngx_http_metrics_handler;
  
  return NGX_OK;
}
  
static void *
ngx_http_metrics_create_main_conf(ngx_conf_t *cf)
{
  fprintf(stderr, "METRICS: call ngx_http_metrics_create_main_conf\n");

  ngx_http_metrics_main_conf_t *mmcf;
  mmcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_metrics_main_conf_t));

  if (mmcf == NULL) {
    return NULL;
  }

  return mmcf;
}

static char *
ngx_http_metrics_enable(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  ngx_http_metrics_main_conf_t *mmcf;
  fprintf(stderr, "METRICS: call ngx_http_metrics_config\n");

  mmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_metrics_module);

  mmcf->enable = 1;
    
  return NGX_OK;
}

static char *
ngx_http_metrics_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  ngx_http_core_loc_conf_t *clcf;

  fprintf(stderr, "METRICS: call ngx_http_metrics_status\n");

  clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
  clcf->handler = ngx_http_metrics_status_handler;
  
  return NGX_CONF_OK;
}

ngx_int_t
ngx_http_metrics_status_handler(ngx_http_request_t *r)
{
  ngx_int_t   rc;
  size_t      size;
  ngx_buf_t   *buf;
  ngx_chain_t out;
  ngx_http_metrics_main_conf_t *mmcf;
    
  if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
    return NGX_HTTP_NOT_ALLOWED;
  }

  rc = ngx_http_discard_request_body(r);

  if (rc != NGX_OK) {
    return rc;
  }

  r->headers_out.content_type_len = sizeof("text/plain") - 1;
  ngx_str_set(&r->headers_out.content_type, "text/plain");
  r->headers_out.content_type_lowcase = NULL;

  if (r->method == NGX_HTTP_HEAD) {
    r->headers_out.status = NGX_HTTP_OK;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
      return rc;
    }
  }

  size = sizeof("Metrics disabled") - 1;

  buf = ngx_create_temp_buf(r->pool, size);

  if (buf == NULL) {
    return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  out.buf  = buf;
  out.next = NULL;

  //TODO: Real metrics
  mmcf = ngx_http_get_module_main_conf(r, ngx_http_metrics_module);
  if (mmcf->enable) {
    buf->last = ngx_cpymem(buf->last, "Metrics enabled", sizeof("Metrics enabled") - 1);
  } else {
    buf->last = ngx_cpymem(buf->last, "Metrics disabled", sizeof("Metrics disabled") - 1);
  }

  r->headers_out.status = NGX_OK;
  r->headers_out.content_length_n = buf->last - buf->pos;

  buf->last_buf = (r == r->main) ? 1 : 0;
  buf->last_in_chain = 1;

  rc = ngx_http_send_header(r);

  if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
    return rc;
  }

  return ngx_http_output_filter(r, &out);
}

ngx_int_t
ngx_http_metrics_handler(ngx_http_request_t *r)
{
  fprintf(stderr, "METRICS: call ngx_http_metrics_end_handler\n");

  ngx_http_metrics_main_conf_t *mmcf;

  mmcf = ngx_http_get_module_main_conf(r, ngx_http_metrics_module);

  if (!mmcf->enable) {
    fprintf(stderr, "METRICS: metrics is disabled\n");
    return NGX_OK;
  }

  //TODO: collect real metrics
  time_t ts = ngx_time();

  fprintf(stderr, "METRICS: Time end = %ld\n", ts);
  
  return NGX_OK;
}
