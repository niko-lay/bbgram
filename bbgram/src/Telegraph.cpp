#include "Telegraph.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

extern "C"
{
#include <tgl-binlog.h>
}

tgl_state* gTLS = 0;

int APP_ID = 14121;
const char* APP_HASH = "bf6bcba5013fe75068e86e2fa7e16594";

const char* RSA_KEY_PATH = "app/native/assets/server.pub";
const char* AUTH_FILE_PATH = "data/auth.dat";
const char* STATE_FILE_PATH = "data/state.dat";
const char* DOWNLOAD_DIRECTORY = "data/downloads";

#define AUTH_FILE_MAGIC 0x868aa81d
#define STATE_FILE_MAGIC 0x28949a93
#define DC_SERIALIZED_MAGIC 0x868aa81d

#include <QDebug>

#include "Storage.h"

static void update_status_notification(struct tgl_state *TLS, struct tgl_user *U)
{
    qDebug() << "update_status_notification" << QString::fromUtf8(U->first_name);
}

static void update_user_registered(struct tgl_state *TLS, struct tgl_user *U)
{
    qDebug() << "update_user_registered" << QString::fromUtf8(U->first_name);
}

static void update_chat_handler (struct tgl_state *TLS, struct tgl_chat *C, unsigned flags)
{
    qDebug() << "update_chat_handler";
}

static void update_user_typing (struct tgl_state *TLS, struct tgl_user *U, enum tgl_typing_status status)
{
    qDebug() << "update_user_typing";
}

void logprintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void write_dc (struct tgl_dc *DC, void *extra)
{
    FILE* auth_file = (FILE*)extra;
    if (!DC)
    {
        int x = 0;
        assert (fwrite(&x, 1, 4, auth_file) == 4);
        return;
    }
    else
    {
        int x = 1;
        assert (fwrite(&x, 1, 4, auth_file) == 4);
    }

    assert (DC->has_auth);

    assert (fwrite(&DC->port, 1, 4, auth_file) == 4);
    int l = strlen (DC->ip);
    assert (fwrite(&l, 1, 4, auth_file) == 4);
    assert (fwrite(DC->ip, 1, l, auth_file) == l);
    assert (fwrite(&DC->auth_key_id, 1, 8, auth_file) == 8);
    assert (fwrite(DC->auth_key, 1, 256, auth_file) == 256);
}

void write_auth_file (struct tgl_state *TLS)
{
    FILE* auth_file = fopen(AUTH_FILE_PATH, "wb");

    if (auth_file == 0)
        return;
    int x = DC_SERIALIZED_MAGIC;
    assert (fwrite(&x, 1, 4, auth_file) == 4);
    assert (fwrite(&TLS->max_dc_num, 1, 4, auth_file) == 4);
    assert (fwrite(&TLS->dc_working_num, 1, 4, auth_file) == 4);

    tgl_dc_iterator_ex (TLS, write_dc, auth_file);

    assert (fwrite(&TLS->our_id, 1, 4, auth_file) == 4);
    fclose (auth_file);
}

void read_dc (struct tgl_state *TLS, FILE* auth_file, int id, unsigned ver)
{
  int port = 0;
  assert (fread(&port, 1, 4, auth_file) == 4);
  int l = 0;
  assert (fread (&l, 1, 4, auth_file) == 4);
  assert (l >= 0 && l < 100);
  char ip[100];
  assert (fread (ip, 1, l, auth_file) == l);
  ip[l] = 0;

  long long auth_key_id;
  static unsigned char auth_key[256];
  assert (fread(&auth_key_id, 1, 8, auth_file) == 8);
  assert (fread(auth_key, 1, 256, auth_file) == 256);

  //bl_do_add_dc (id, ip, l, port, auth_key_id, auth_key);
  bl_do_dc_option (TLS, id, 2, "DC", l, ip, port);
  bl_do_set_auth_key_id (TLS, id, auth_key);
  bl_do_dc_signed (TLS, id);
}

bool read_auth_file (struct tgl_state *TLS)
{
    FILE* auth_file = fopen(AUTH_FILE_PATH, "rb");
    if (auth_file == 0)
        return false;

    unsigned x;
    unsigned m;
    if (fread (&m, 1, 4, auth_file) < 4 || (m != AUTH_FILE_MAGIC))
    {
        fclose (auth_file);
        return false;
    }
    assert (fread (&x, 1, 4, auth_file) == 4);
    assert (x > 0);
    int dc_working_num;
    assert (fread (&dc_working_num, 1, 4, auth_file) == 4);

    int i;
    for (i = 0; i <= (int)x; i++)
    {
        int y;
        assert (fread (&y, 1, 4, auth_file) == 4);
        if (y)
        {
            read_dc(TLS, auth_file, i, m);
        }
    }
    bl_do_set_working_dc(TLS, dc_working_num);
    int our_id;
    int l = fread (&our_id, 1, 4, auth_file);
    if (l < 4)
    {
        assert (!l);
    }
    if (our_id)
    {
        bl_do_set_our_id(TLS, our_id);
    }
    fclose(auth_file);
    return true;
}

void empty_auth_file (struct tgl_state *TLS)
{
    if (TLS->test_mode)
    {
        bl_do_dc_option (TLS, 1, 0, "", strlen (TG_SERVER_TEST_1), TG_SERVER_TEST_1, 443);
        bl_do_dc_option (TLS, 2, 0, "", strlen (TG_SERVER_TEST_2), TG_SERVER_TEST_2, 443);
        bl_do_dc_option (TLS, 3, 0, "", strlen (TG_SERVER_TEST_3), TG_SERVER_TEST_3, 443);
        bl_do_set_working_dc (TLS, TG_SERVER_TEST_DEFAULT);
    }
    else
    {
        bl_do_dc_option (TLS, 1, 0, "", strlen (TG_SERVER_1), TG_SERVER_1, 443);
        bl_do_dc_option (TLS, 2, 0, "", strlen (TG_SERVER_2), TG_SERVER_2, 443);
        bl_do_dc_option (TLS, 3, 0, "", strlen (TG_SERVER_3), TG_SERVER_3, 443);
        bl_do_dc_option (TLS, 4, 0, "", strlen (TG_SERVER_4), TG_SERVER_4, 443);
        bl_do_dc_option (TLS, 5, 0, "", strlen (TG_SERVER_5), TG_SERVER_5, 443);
        bl_do_set_working_dc (TLS, TG_SERVER_DEFAULT);
    }
}

void write_state_file (struct tgl_state *TLS)
{
  int wseq = TLS->seq;
  int wpts = TLS->pts;
  int wqts = TLS->qts;
  int wdate = TLS->date;

  FILE* state_file = fopen(STATE_FILE_PATH, "wb");

  if (state_file == 0)
      return;

  int x[6];
  x[0] = STATE_FILE_MAGIC;
  x[1] = 0;
  x[2] = wpts;
  x[3] = wqts;
  x[4] = wseq;
  x[5] = wdate;
  assert (fwrite (x, 1, 24, state_file) == 24);
  fclose (state_file);
}

bool read_state_file (struct tgl_state *TLS)
{
    FILE* state_file = fopen(STATE_FILE_PATH, "rb");

    if (state_file == 0)
        return false;

    int version, magic;
    if (fread(&magic, 1, 4, state_file) < 4) { fclose(state_file); return false; }
    if (magic != (int)STATE_FILE_MAGIC) { fclose(state_file); return false; }
    if (fread(&version, 1, 4, state_file) < 4 || version < 0) { fclose(state_file); return false; }
    int x[4];
    if (fread(x, 1, 16, state_file) < 16) {
        fclose(state_file);
        return false;
    }
    int pts = x[0];
    int qts = x[1];
    int seq = x[2];
    int date = x[3];
    fclose(state_file);
    bl_do_set_seq(TLS, seq);
    bl_do_set_pts(TLS, pts);
    bl_do_set_qts(TLS, qts);
    bl_do_set_date(TLS, date);
    return true;
}

#include "telegraph/timers.h"
#include "telegraph/net.h"


Telegraph* Telegraph::m_instance = 0;

Telegraph::Telegraph()
{
    m_instance = this;

    memset(&m_updateCallbacks, 0, sizeof(tgl_update_callback));
}

Telegraph::~Telegraph()
{
    m_instance = 0;
}

Telegraph* Telegraph::instance()
{
    return m_instance;
}

#include <QDebug>

tgl_timer_methods timer_methods;
tgl_net_methods net_methods;

bool e = false;
void Telegraph::checkAllAuthorized()
{
    if (!e && gTLS->DC_working && tgl_authorized_dc(gTLS, gTLS->DC_working))
    {
        e =true;
        emit mainDCAuthorized();
    }
    for (int i = 0; i <= gTLS->max_dc_num; i++)
    {
        if (gTLS->DC_list[i])
        {
            if (!tgl_authorized_dc(gTLS, gTLS->DC_list[i]))
            {
                //qDebug() << i << "/" << gTLS->max_dc_num << "dc not authorized :(";
                return;
            }
        }
    }

    qDebug() << "All dc authorized!";
    emit allDCAuthorized();
    m_checkAllAuthorizedTimer->stop();
    delete m_checkAllAuthorizedTimer;
    m_checkAllAuthorizedTimer = 0;
}

void Telegraph::start()
{
    m_updateCallbacks.logprintf = logprintf;
    m_updateCallbacks.user_update = Storage::userUpdateHandler;
    m_updateCallbacks.user_status_update = Storage::userStatusUpdateHandler;
    m_updateCallbacks.msg_receive = Storage::messageReceivedHandler;
    m_updateCallbacks.status_notification = update_status_notification;
    m_updateCallbacks.chat_update = update_chat_handler;
    m_updateCallbacks.type_notification = update_user_typing;
    m_updateCallbacks.user_registered = update_user_registered;


    memset(&timer_methods, 0, sizeof(tgl_timer_methods));
    timer_methods.alloc = alloc_timer;
    timer_methods.insert = insert_timer;
    timer_methods.remove = remove_timer;
    timer_methods.free = free_timer;

    memset(&net_methods, 0, sizeof(tgl_net_methods));
    net_methods.create_connection = connection_create;
    net_methods.write_out = connection_write_out;
    net_methods.read_in = connection_read_in;
    net_methods.read_in_lookup = connection_read_in_lookup;
    net_methods.flush_out = connection_flush_out;
    net_methods.incr_out_packet_num = connection_incr_out_packet_num;
    net_methods.free = connection_free;
    net_methods.get_dc = connection_get_dc;
    net_methods.get_session = connection_get_session;

    gTLS = tgl_state_alloc();
    //gTLS->test_mode = 1;

    tgl_set_verbosity(gTLS, 0);
    tgl_set_rsa_key(gTLS, RSA_KEY_PATH);

    //event_base* ev = event_base_new();
    //tgl_set_ev_base(gTLS, ev);
    /*char download_dir[256];
    getcwd(download_dir, 256);
    strcat(download_dir, "/");
    strcat(download_dir, DOWNLOAD_DIRECTORY);*/
    int r = mkdir(DOWNLOAD_DIRECTORY, 0700);
    tgl_set_download_directory(gTLS, DOWNLOAD_DIRECTORY);

    tgl_set_net_methods(gTLS, &net_methods);
    tgl_set_timer_methods(gTLS, &timer_methods);
    tgl_set_callback(gTLS, &m_updateCallbacks);
    tgl_register_app_id(gTLS, APP_ID, (char*)APP_HASH);

    tgl_init(gTLS);

    if (!read_auth_file(gTLS))
        empty_auth_file(gTLS);
    read_state_file(gTLS);


    m_checkAllAuthorizedTimer = new QTimer(this);
    connect(m_checkAllAuthorizedTimer, SIGNAL(timeout()), this, SLOT(checkAllAuthorized()));
    m_checkAllAuthorizedTimer->start(100);

    m_writeStateTimer = new QTimer(this);
    connect(m_writeStateTimer, SIGNAL(timeout()), this, SLOT(writeState()));
}

void Telegraph::stop()
{
    tgl_free_all(gTLS);
    gTLS = 0;
}

void export_auth_callback (struct tgl_state *TLS, void *extra, int success)
{
    assert (success);
}

void Telegraph::exportAuthorization()
{
    static tgl_dc *cur_a_dc = 0;
    if (cur_a_dc && !tgl_signed_dc(gTLS, cur_a_dc))
    {
        QTimer::singleShot(50, this, SLOT(exportAuthorization()));
        return;
    }
    cur_a_dc = 0;

    for (int i = 0; i <= gTLS->max_dc_num; i++)
            if (gTLS->DC_list[i] && !tgl_signed_dc (gTLS, gTLS->DC_list[i]))
            {
                tgl_do_export_auth (gTLS, i, export_auth_callback, (void*)(long)gTLS->DC_list[i]);
                cur_a_dc = gTLS->DC_list[i];
                QTimer::singleShot(50, this, SLOT(exportAuthorization()));
                return;
            }
        write_auth_file(gTLS);
        Telegraph::instance()->onReady();
}

void Telegraph::onReady()
{
    emit ready();
    m_writeStateTimer->start(5000);
}

void Telegraph::writeState()
{
    write_state_file(gTLS);
}
