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
 * Copyright 2021, Wabi.
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

#include "KLI_string_utils.h"
#include "KLI_utildefines.h"

#include "KKE_main.h"
#include "KKE_report.h"

WABI_NAMESPACE_BEGIN

const char *KKE_report_type_str(eReportType type)
{
  switch (type)
  {
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

char *KKE_reports_string(ReportList *reports, eReportType level)
{
  std::string text = std::string();
  char *cstring;

  if (!reports || !(*reports->list.begin()))
  {
    return NULL;
  }

  UNIVERSE_FOR_ALL(rep, reports->list)
  {
    if (rep->type >= level)
    {
      text = TfStringPrintf("%s: %s\n", rep->typestr, rep->message);
    }
  }

  if (!text.empty())
  {
    KLI_strncpy(cstring, CHARALL(text), sizeof(CHARALL(text)));
  }
  else
  {
    cstring = NULL;
  }

  return cstring;
}

void KKE_reports_init(ReportList *reports, int flag)
{
  if (!reports)
  {
    return;
  }

  memset(reports, 0, sizeof(ReportList));

  reports->storelevel = RPT_INFO;
  reports->printlevel = RPT_ERROR;
  reports->flag = flag;
}

void KKE_reports_clear(ReportList *reports)
{
  if (!reports)
  {
    return;
  }

  auto report = reports->list.begin();

  while (report != reports->list.end())
  {
    free((void *)(*report)->message);
    delete (*report);
    report++;
  }

  reports->list.clear();
}

void KKE_report(ReportList *reports, eReportType type, const char *_message)
{
  Report *report;
  int len;
  const char *message = TIP_(_message);

  if (G.background || !reports || ((reports->flag & RPT_PRINT) && (type >= reports->printlevel)))
  {
    printf("%s: %s\n", KKE_report_type_str(type), message);
    fflush(stdout);
  }

  if (reports && (reports->flag & RPT_STORE) && (type >= reports->storelevel))
  {
    char *message_alloc;
    report = new Report();
    report->type = type;
    report->typestr = KKE_report_type_str(type);

    len = strlen(message);
    message_alloc = (char *)malloc(sizeof(char) * (len + 1));
    memcpy(message_alloc, message, sizeof(char) * (len + 1));
    report->message = message_alloc;
    report->len = len;

    reports->list.push_back(report);
  }
}

void KKE_reportf(ReportList *reports, eReportType type, const char *_format, ...)
{
  std::string text;
  Report *report;
  va_list args;
  const char *format = TIP_(_format);

  if (G.background || !reports || ((reports->flag & RPT_PRINT) && (type >= reports->printlevel)))
  {
    printf("%s: ", KKE_report_type_str(type));
    va_start(args, _format);
    vprintf(format, args);
    va_end(args);
    fprintf(stdout, "\n");
    fflush(stdout);
  }

  if (reports && (reports->flag & RPT_STORE) && (type >= reports->storelevel))
  {

    report = new Report();

    va_start(args, _format);
    text = TfVStringPrintf(format, args);
    va_end(args);

    report->message = CHARALL(text);
    report->len = text.length();

    report->type = type;
    report->typestr = KKE_report_type_str(type);

    reports->list.push_back(report);
  }
}

WABI_NAMESPACE_END