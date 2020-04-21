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
#include "slidepulseaudiomonitor.h"

#ifdef HAVE_PULSE
    #include <context.h>
    #include <pulse/glib-mainloop.h>
#endif
#include <tpromise.h>

struct SlidePulseaudioMonitorPrivate {
    pa_context* ctx;
    QString defaultSinkName;

#ifdef HAVE_PULSE
    PulseAudioQt::Sink* defaultSink;
#endif

    pa_stream* monitoringStream = nullptr;
    QByteArray dataBuf;
};

SlidePulseaudioMonitor::SlidePulseaudioMonitor(QObject* parent) : QObject(parent) {
#ifdef HAVE_PULSE
    d = new SlidePulseaudioMonitorPrivate();

    //Connect to Pulse
    pa_proplist* proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, QObject::tr("theShell Screen Lock").toUtf8().constData());
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "com.vicr123.tsscreenlock");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");

    d->ctx = pa_context_new_with_proplist(pa_glib_mainloop_get_api(pa_glib_mainloop_new(g_main_context_default())), nullptr, proplist);
    pa_proplist_free(proplist);

    pa_context_set_state_callback(d->ctx, &SlidePulseaudioMonitor::context_callback, this);
    pa_context_connect(d->ctx, nullptr, PA_CONTEXT_NOFAIL, nullptr);
#endif
}

SlidePulseaudioMonitor::~SlidePulseaudioMonitor() {
#ifdef HAVE_PULSE
    pa_context_disconnect(d->ctx);
    delete d;
#endif
}

void SlidePulseaudioMonitor::dataAvailable() {
    const ulong bufReadSize = sizeof(float) * 1024;
    while (static_cast<unsigned long>(d->dataBuf.length()) >= bufReadSize) {
        const float* data = reinterpret_cast<const float*>(d->dataBuf.constData());
        emit audioDataAvailable(data, 1024);

        d->dataBuf.remove(0, bufReadSize);
    }
}


#ifdef HAVE_PULSE
void SlidePulseaudioMonitor::defaultSinkChanged(pa_sink_info defaultSink) {
    if (d->monitoringStream) {
        pa_stream_disconnect(d->monitoringStream);
        pa_stream_unref(d->monitoringStream);
    }

    pa_buffer_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.fragsize = sizeof(float);
    attr.maxlength = static_cast<quint32>(-1);

    pa_sample_spec spec;
    spec.channels = 1;
    spec.format = PA_SAMPLE_FLOAT32;
//    spec.rate = 16384;
    spec.rate = 32768;

    char t[16];
    snprintf(t, sizeof(t), "%u", defaultSink.monitor_source);

    d->monitoringStream = pa_stream_new(d->ctx, qPrintable(tr("Lock Screen Visualisation")), &spec, nullptr);
    if (!d->monitoringStream) {
        //Error!
        return;
    }

    pa_stream_set_read_callback(d->monitoringStream, &read_callback, this);
    if (pa_stream_connect_record(d->monitoringStream, t, &attr,
            static_cast<pa_stream_flags_t>(PA_STREAM_DONT_MOVE | PA_STREAM_ADJUST_LATENCY)) < 0) {
        //Error!
        pa_stream_unref(d->monitoringStream);
        d->monitoringStream = nullptr;
    }
}

void SlidePulseaudioMonitor::sink_callback(pa_context* c, const pa_sink_info* i, int eol, void* userdata) {
    if (eol < 0) return;
    if (!i) return;

    SlidePulseaudioMonitor* me = reinterpret_cast<SlidePulseaudioMonitor*>(userdata);
    me->defaultSinkChanged(*i);
}

void SlidePulseaudioMonitor::read_callback(pa_stream* s, size_t length, void* userdata) {
    SlidePulseaudioMonitor* me = reinterpret_cast<SlidePulseaudioMonitor*>(userdata);

    const void* data;
//    double v;

    if (pa_stream_peek(s, &data, &length) < 0) {
        //Error!
        return;
    }

    if (!data) {
        //We've got a hole or an empty buffer; drop it if it's a hole
        if (length > 0) pa_stream_drop(s);
        return;
    }

    me->d->dataBuf.append(reinterpret_cast<const char*>(data), static_cast<int>(length));
    me->dataAvailable();
    pa_stream_drop(s);
}

void SlidePulseaudioMonitor::context_callback(pa_context* c, void* userdata) {
    if (pa_context_get_state(c) == PA_CONTEXT_READY) {
        pa_context_get_server_info(c, &SlidePulseaudioMonitor::server_info_callback, userdata);
    }
}

void SlidePulseaudioMonitor::server_info_callback(pa_context* c, const pa_server_info* i, void* userdata) {
    SlidePulseaudioMonitor* me = reinterpret_cast<SlidePulseaudioMonitor*>(userdata);
    if (me->d->defaultSinkName != i->default_sink_name) {
        me->d->defaultSinkName = i->default_sink_name;
        pa_context_get_sink_info_by_name(c, i->default_sink_name, &SlidePulseaudioMonitor::sink_callback, userdata);
    }
}
#endif
