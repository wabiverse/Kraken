/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "MEM_guardedalloc.h"

#include "KLI_fileops.h"
#include "KLI_listbase.h"
#include "KLI_dynstr.h"
#include "KLI_utildefines.h"

#include "KKE_main.h"
#include "KKE_report.h"
#include "KKE_global.h"

const char *KKE_report_type_str(eReportType type)
{
  switch (type) {
    case RPT_DEBUG:
      return TIP_("Debug");
    case RPT_INFO:
      return TIP_("Info");
    case RPT_OPERATOR:
      return TIP_("Operator");
    case RPT_PROPERTY:
      return TIP_("Property");
    case RPT_WARNING:
      return TIP_("Warning");
    case RPT_ERROR:
      return TIP_("Error");
    case RPT_ERROR_INVALID_INPUT:
      return TIP_("Invalid Input Error");
    case RPT_ERROR_INVALID_CONTEXT:
      return TIP_("Invalid Context Error");
    case RPT_ERROR_OUT_OF_MEMORY:
      return TIP_("Out Of Memory Error");
    default:
      return TIP_("Undefined Type");
  }
}

void KKE_reports_init(ReportList *reports, int flag)
{
  if (!reports) {
    return;
  }

  memset(reports, 0, sizeof(ReportList));

  reports->storelevel = RPT_INFO;
  reports->printlevel = RPT_ERROR;
  reports->flag = flag;
}

void KKE_reports_clear(ReportList *reports)
{
  Report *report, *report_next;

  if (!reports) {
    return;
  }

  report = (Report *)reports->list.first;

  while (report) {
    report_next = report->next;
    MEM_freeN((void *)report->message);
    MEM_freeN(report);
    report = report_next;
  }

  KLI_listbase_clear(&reports->list);
}

void KKE_report(ReportList *reports, eReportType type, const char *_message)
{
  Report *report;
  int len;
  const char *message = TIP_(_message);

  if (KKE_reports_print_test(reports, type)) {
    printf("%s: %s\n", KKE_report_type_str(type), message);
    fflush(stdout); /* this ensures the message is printed before a crash */
  }

  if (reports && (reports->flag & RPT_STORE) && (type >= reports->storelevel)) {
    char *message_alloc;
    report = (Report *)MEM_callocN(sizeof(Report), "Report");
    report->type = type;
    report->typestr = KKE_report_type_str(type);

    len = strlen(message);
    message_alloc = (char *)MEM_mallocN(sizeof(char) * (len + 1), "ReportMessage");
    memcpy(message_alloc, message, sizeof(char) * (len + 1));
    report->message = message_alloc;
    report->len = len;
    KLI_addtail(&reports->list, report);
  }
}

void KKE_reportf(ReportList *reports, eReportType type, const char *_format, ...)
{
  DynStr *ds;
  Report *report;
  va_list args;
  const char *format = TIP_(_format);

  if (KKE_reports_print_test(reports, type)) {
    printf("%s: ", KKE_report_type_str(type));
    va_start(args, _format);
    vprintf(format, args);
    va_end(args);
    fprintf(stdout, "\n"); /* otherwise each report needs to include a \n */
    fflush(stdout);        /* this ensures the message is printed before a crash */
  }

  if (reports && (reports->flag & RPT_STORE) && (type >= reports->storelevel)) {
    report = (Report *)MEM_callocN(sizeof(Report), "Report");

    ds = KLI_dynstr_new();
    va_start(args, _format);
    KLI_dynstr_vappendf(ds, format, args);
    va_end(args);

    report->message = KLI_dynstr_get_cstring(ds);
    report->len = KLI_dynstr_get_len(ds);
    KLI_dynstr_free(ds);

    report->type = type;
    report->typestr = KKE_report_type_str(type);

    KLI_addtail(&reports->list, report);
  }
}

void KKE_reports_prepend(ReportList *reports, const char *_prepend)
{
  Report *report;
  DynStr *ds;
  const char *prepend = TIP_(_prepend);

  if (!reports) {
    return;
  }

  for (report = (Report *)reports->list.first; report; report = report->next) {
    ds = KLI_dynstr_new();

    KLI_dynstr_append(ds, prepend);
    KLI_dynstr_append(ds, report->message);
    MEM_freeN((void *)report->message);

    report->message = KLI_dynstr_get_cstring(ds);
    report->len = KLI_dynstr_get_len(ds);

    KLI_dynstr_free(ds);
  }
}

void KKE_reports_prependf(ReportList *reports, const char *_prepend, ...)
{
  Report *report;
  DynStr *ds;
  va_list args;
  const char *prepend = TIP_(_prepend);

  if (!reports) {
    return;
  }

  for (report = (Report *)reports->list.first; report; report = report->next) {
    ds = KLI_dynstr_new();
    va_start(args, _prepend);
    KLI_dynstr_vappendf(ds, prepend, args);
    va_end(args);

    KLI_dynstr_append(ds, report->message);
    MEM_freeN((void *)report->message);

    report->message = KLI_dynstr_get_cstring(ds);
    report->len = KLI_dynstr_get_len(ds);

    KLI_dynstr_free(ds);
  }
}

eReportType KKE_report_print_level(ReportList *reports)
{
  if (!reports) {
    return RPT_ERROR;
  }

  return static_cast<eReportType>(reports->printlevel);
}

void KKE_report_print_level_set(ReportList *reports, eReportType level)
{
  if (!reports) {
    return;
  }

  reports->printlevel = level;
}

eReportType KKE_report_store_level(ReportList *reports)
{
  if (!reports) {
    return RPT_ERROR;
  }

  return static_cast<eReportType>(reports->storelevel);
}

void KKE_report_store_level_set(ReportList *reports, eReportType level)
{
  if (!reports) {
    return;
  }

  reports->storelevel = level;
}

char *KKE_reports_string(ReportList *reports, eReportType level)
{
  Report *report;
  DynStr *ds;
  char *cstring;

  if (!reports || !reports->list.first) {
    return NULL;
  }

  ds = KLI_dynstr_new();
  for (report = (Report *)reports->list.first; report; report = report->next) {
    if (report->type >= level) {
      KLI_dynstr_appendf(ds, "%s: %s\n", report->typestr, report->message);
    }
  }

  if (KLI_dynstr_get_len(ds)) {
    cstring = KLI_dynstr_get_cstring(ds);
  } else {
    cstring = NULL;
  }

  KLI_dynstr_free(ds);
  return cstring;
}

bool KKE_reports_print_test(const ReportList *reports, eReportType type)
{
  if (reports == NULL) {
    return true;
  }
  if (reports->flag & RPT_PRINT_HANDLED_BY_OWNER) {
    return false;
  }
  /* In background mode always print otherwise there are cases the errors won't be displayed,
   * but still add to the report list since this is used for Python exception handling. */
  if (G.background) {
    return true;
  }

  /* Common case. */
  return (reports->flag & RPT_PRINT) && (type >= reports->printlevel);
}

void KKE_reports_print(ReportList *reports, eReportType level)
{
  char *cstring = KKE_reports_string(reports, level);

  if (cstring == NULL) {
    return;
  }

  puts(cstring);
  fflush(stdout);
  MEM_freeN(cstring);
}

Report *KKE_reports_last_displayable(ReportList *reports)
{
  Report *report;

  for (report = (Report *)reports->list.last; report; report = report->prev) {
    if (ELEM(report->type, RPT_ERROR, RPT_WARNING, RPT_INFO)) {
      return report;
    }
  }

  return NULL;
}

bool KKE_reports_contain(ReportList *reports, eReportType level)
{
  Report *report;
  if (reports != NULL) {
    for (report = (Report *)reports->list.first; report; report = report->next) {
      if (report->type >= level) {
        return true;
      }
    }
  }
  return false;
}

bool KKE_report_write_file_fp(FILE *fp, ReportList *reports, const char *header)
{
  Report *report;

  if (header) {
    fputs(header, fp);
  }

  for (report = (Report *)reports->list.first; report; report = report->next) {
    fprintf((FILE *)fp, "%s  # %s\n", report->message, report->typestr);
  }

  return true;
}

bool KKE_report_write_file(const char *filepath, ReportList *reports, const char *header)
{
  FILE *fp;

  errno = 0;
  fp = KLI_fopen(filepath, "wb");
  if (fp == NULL) {
    fprintf(stderr,
            "Unable to save '%s': %s\n",
            filepath,
            errno ? strerror(errno) : "Unknown error opening file");
    return false;
  }

  KKE_report_write_file_fp(fp, reports, header);

  fclose(fp);

  return true;
}
