/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#ifndef SLIDEPULSEAUDIOMONITOR_H
#define SLIDEPULSEAUDIOMONITOR_H

#include <QObject>

#ifdef HAVE_PULSE
    #include <pulse/pulseaudio.h>
#endif

class pa_stream;
class pa_context;
class pa_server_info;
struct SlidePulseaudioMonitorPrivate;
class SlidePulseaudioMonitor : public QObject {
        Q_OBJECT
    public:
        explicit SlidePulseaudioMonitor(QObject* parent = nullptr);
        ~SlidePulseaudioMonitor();

    signals:
        void audioDataAvailable(const float* points, int length);

    private slots:
        void dataAvailable();

#ifdef HAVE_PULSE
        void defaultSinkChanged(pa_sink_info defaultSink);
#endif

    private:
#ifdef HAVE_PULSE
        static void sink_callback(pa_context* c, const pa_sink_info* i, int eol, void* userdata);
        static void read_callback(pa_stream* s, size_t length, void* userdata);
        static void context_callback(pa_context* c, void* userdata);
        static void server_info_callback(pa_context* c, const pa_server_info* i, void* userdata);
#endif
        SlidePulseaudioMonitorPrivate* d;
};

#endif // SLIDEPULSEAUDIOMONITOR_H
