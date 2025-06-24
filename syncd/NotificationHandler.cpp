#include "NotificationHandler.h"
#include "sairediscommon.h"

#include "swss/logger.h"

#include "meta/sai_serialize.h"

#include <inttypes.h>

using namespace syncd;

NotificationHandler::NotificationHandler(
        _In_ std::shared_ptr<NotificationProcessor> processor):
    m_processor(processor)
{
    SWSS_LOG_ENTER();

    memset(&m_switchNotifications, 0, sizeof(m_switchNotifications));

    m_notificationQueue = processor->getQueue();
}

NotificationHandler::~NotificationHandler()
{
    SWSS_LOG_ENTER();

    // empty
}

void NotificationHandler::setSwitchNotifications(
        _In_ const sai_switch_notifications_t& switchNotifications)
{
    SWSS_LOG_ENTER();

    m_switchNotifications = switchNotifications;
}

const sai_switch_notifications_t& NotificationHandler::getSwitchNotifications() const
{
    SWSS_LOG_ENTER();

    return m_switchNotifications;
}

void NotificationHandler::updateNotificationsPointers(
        _In_ uint32_t attr_count,
        _In_ sai_attribute_t *attr_list) const
{
    SWSS_LOG_ENTER();

    /*
     * This function should only be called on CREATE/SET api when object is
     * SWITCH.
     *
     * Notifications pointers needs to be corrected since those we receive from
     * sairedis are in sairedis memory space and here we are using those ones
     * we declared in syncd memory space.
     *
     * Also notice that we are using the same pointers for ALL switches.
     */

    for (uint32_t index = 0; index < attr_count; ++index)
    {
        sai_attribute_t &attr = attr_list[index];

        auto meta = sai_metadata_get_attr_metadata(SAI_OBJECT_TYPE_SWITCH, attr.id);

        if (meta->attrvaluetype != SAI_ATTR_VALUE_TYPE_POINTER)
        {
            continue;
        }

        /*
         * Does not matter if pointer is valid or not, we just want the
         * previous value.
         */

        sai_pointer_t prev = attr.value.ptr;

        if (prev == NULL)
        {
            /*
             * If pointer is NULL, then fine, let it be.
             */

            continue;
        }

        switch (attr.id)
        {
            case SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_switch_state_change;
                break;

            case SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_switch_shutdown_request;
                break;

            case SAI_SWITCH_ATTR_SWITCH_ASIC_SDK_HEALTH_EVENT_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_switch_asic_sdk_health_event;
                break;

            case SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_fdb_event;
                break;

            case SAI_SWITCH_ATTR_NAT_EVENT_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_nat_event;
                break;

            case SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_port_state_change;
                break;

            case SAI_SWITCH_ATTR_PORT_HOST_TX_READY_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_port_host_tx_ready;
                break;

            case SAI_SWITCH_ATTR_QUEUE_PFC_DEADLOCK_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_queue_pfc_deadlock;
                break;

            case SAI_SWITCH_ATTR_BFD_SESSION_STATE_CHANGE_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_bfd_session_state_change;
                break;

            case SAI_SWITCH_ATTR_TWAMP_SESSION_EVENT_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_twamp_session_event;
                break;

            case SAI_SWITCH_ATTR_SWITCH_MACSEC_POST_STATUS_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_switch_macsec_post_status;
                break;

            case SAI_SWITCH_ATTR_SWITCH_IPSEC_POST_STATUS_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_switch_ipsec_post_status;
                break;

            case SAI_SWITCH_ATTR_MACSEC_POST_STATUS_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_macsec_post_status;
                break;

            case SAI_SWITCH_ATTR_IPSEC_POST_STATUS_NOTIFY:
                attr.value.ptr = (void*)m_switchNotifications.on_ipsec_post_status;
                break;

            default:

                SWSS_LOG_ERROR("pointer for %s is not handled, FIXME!", meta->attridname);
                continue;
        }

        // Here we translated pointer, just log it.

        SWSS_LOG_INFO("%s: 0x%" PRIx64 " (orch) => 0x%" PRIx64 " (syncd)", meta->attridname, (uint64_t)prev, (uint64_t)attr.value.ptr);
    }
}

// TODO use same Notification class from sairedis lib
// then this will handle deserialize free

void NotificationHandler::onFdbEvent(
        _In_ uint32_t count,
        _In_ const sai_fdb_event_notification_data_t *data)
{
    SWSS_LOG_ENTER();

    std::string s = sai_serialize_fdb_event_ntf(count, data);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_FDB_EVENT, s);
}

void NotificationHandler::onNatEvent(
        _In_ uint32_t count,
        _In_ const sai_nat_event_notification_data_t *data)
{
    SWSS_LOG_ENTER();

    std::string s = sai_serialize_nat_event_ntf(count, data);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_NAT_EVENT, s);
}

void NotificationHandler::onPortStateChange(
        _In_ uint32_t count,
        _In_ const sai_port_oper_status_notification_t *data)
{
    SWSS_LOG_ENTER();

    auto s = sai_serialize_port_oper_status_ntf(count, data);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_PORT_STATE_CHANGE, s);
}

void NotificationHandler::onPortHostTxReady(
        _In_ sai_object_id_t switch_id,
        _In_ sai_object_id_t port_id,
        _In_ sai_port_host_tx_ready_status_t host_tx_ready_status)
{
    SWSS_LOG_ENTER();

    auto s = sai_serialize_port_host_tx_ready_ntf(switch_id, port_id, host_tx_ready_status);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_PORT_HOST_TX_READY, s);
}

void NotificationHandler::onQueuePfcDeadlock(
        _In_ uint32_t count,
        _In_ const sai_queue_deadlock_notification_data_t *data)
{
    SWSS_LOG_ENTER();

    auto s = sai_serialize_queue_deadlock_ntf(count, data);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_QUEUE_PFC_DEADLOCK, s);
}

void NotificationHandler::onSwitchShutdownRequest(
        _In_ sai_object_id_t switch_id)
{
    SWSS_LOG_ENTER();

    auto s = sai_serialize_switch_shutdown_request(switch_id);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_SWITCH_SHUTDOWN_REQUEST, s);
}

void  NotificationHandler::onSwitchAsicSdkHealthEvent(
        _In_ sai_object_id_t switch_id,
        _In_ sai_switch_asic_sdk_health_severity_t severity,
        _In_ sai_timespec_t timestamp,
        _In_ sai_switch_asic_sdk_health_category_t category,
        _In_ sai_switch_health_data_t data,
        _In_ const sai_u8_list_t description)
{
    SWSS_LOG_ENTER();

    std::string s = sai_serialize_switch_asic_sdk_health_event(switch_id, severity, timestamp, category, data, description);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_SWITCH_ASIC_SDK_HEALTH_EVENT, s);
}

void NotificationHandler::onSwitchStateChange(
        _In_ sai_object_id_t switch_id,
        _In_ sai_switch_oper_status_t switch_oper_status)
{
    SWSS_LOG_ENTER();

    auto s = sai_serialize_switch_oper_status(switch_id, switch_oper_status);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_SWITCH_STATE_CHANGE, s);
}

void NotificationHandler::onBfdSessionStateChange(
        _In_ uint32_t count,
        _In_ const sai_bfd_session_state_notification_t *data)
{
    SWSS_LOG_ENTER();

    std::string s = sai_serialize_bfd_session_state_ntf(count, data);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_BFD_SESSION_STATE_CHANGE, s);
}

void NotificationHandler::enqueueNotification(
        _In_ const std::string& op,
        _In_ const std::string& data,
        _In_ const std::vector<swss::FieldValueTuple> &entry)
{
    SWSS_LOG_ENTER();

    SWSS_LOG_INFO("%s %s", op.c_str(), data.c_str());

    swss::KeyOpFieldsValuesTuple item(op, data, entry);

    if (m_notificationQueue->enqueue(item))
    {
        m_processor->signal();
    }
}

void NotificationHandler::onTwampSessionEvent(
        _In_ uint32_t count,
        _In_ const sai_twamp_session_event_notification_data_t *data)
{
    SWSS_LOG_ENTER();

    std::string s = sai_serialize_twamp_session_event_ntf(count, data);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_TWAMP_SESSION_EVENT, s);
}

void NotificationHandler::onMacsecPostStatus(
    _In_ sai_object_id_t macsec_id,
    _In_ sai_macsec_post_status_t macsec_post_status)
{
    SWSS_LOG_ENTER();

    auto s = sai_serialize_macsec_post_status(macsec_id, macsec_post_status);
    SWSS_LOG_WARN("wumiao onMacsecPostStatus");
    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_MACSEC_POST_STATUS, s);
}

void NotificationHandler::onSwitchMacsecPostStatus(
    _In_ sai_object_id_t switch_id,
    _In_ sai_switch_macsec_post_status_t switch_macsec_post_status)
{
    SWSS_LOG_ENTER();
    SWSS_LOG_WARN("wumiao onSwitchMacsecPostStatus");
    auto s = sai_serialize_switch_macsec_post_status(switch_id, switch_macsec_post_status);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_SWITCH_MACSEC_POST_STATUS, s);
}

void NotificationHandler::onIpsecPostStatus(
    _In_ sai_object_id_t ipsec_id,
    _In_ sai_ipsec_post_status_t ipsec_post_status)
{
    SWSS_LOG_ENTER();

    auto s = sai_serialize_ipsec_post_status(ipsec_id, ipsec_post_status);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_IPSEC_POST_STATUS, s);
}

void NotificationHandler::onSwitchIpsecPostStatus(
    _In_ sai_object_id_t switch_id,
    _In_ sai_switch_ipsec_post_status_t switch_ipsec_post_status)
{
    SWSS_LOG_ENTER();

    auto s = sai_serialize_switch_ipsec_post_status(switch_id, switch_ipsec_post_status);

    enqueueNotification(SAI_SWITCH_NOTIFICATION_NAME_SWITCH_IPSEC_POST_STATUS, s);
}

void NotificationHandler::enqueueNotification(
        _In_ const std::string& op,
        _In_ const std::string& data)
{
    SWSS_LOG_ENTER();

    std::vector<swss::FieldValueTuple> entry;

    enqueueNotification(op, data, entry);
}