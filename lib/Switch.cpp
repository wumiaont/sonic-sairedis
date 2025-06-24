#include "Switch.h"

#include "meta/Globals.h"

#include "swss/logger.h"

#include <cstring>

using namespace sairedis;

Switch::Switch(
        _In_ sai_object_id_t switchId):
    m_switchId(switchId)
{
    SWSS_LOG_ENTER();

    if (switchId == SAI_NULL_OBJECT_ID)
    {
        SWSS_LOG_THROW("switch id can't be NULL");
    }

    clearNotificationsPointers();
}

Switch::Switch(
        _In_ sai_object_id_t switchId,
        _In_ uint32_t attrCount,
        _In_ const sai_attribute_t *attrList):
    Switch(switchId)
{
    SWSS_LOG_ENTER();

    updateNotifications(attrCount, attrList);

    // SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO is create only attribute

    m_hardwareInfo = saimeta::Globals::getHardwareInfo(attrCount, attrList);

    SWSS_LOG_NOTICE("created switch with hwinfo = '%s'", m_hardwareInfo.c_str());
}

void Switch::clearNotificationsPointers()
{
    SWSS_LOG_ENTER();

    memset(&m_switchNotifications, 0, sizeof(m_switchNotifications));
}

sai_object_id_t Switch::getSwitchId() const
{
    SWSS_LOG_ENTER();

    return m_switchId;
}

void Switch::updateNotifications(
        _In_ uint32_t attrCount,
        _In_ const sai_attribute_t *attrList)
{
    SWSS_LOG_ENTER();

    /*
     * This function should only be called on CREATE/SET
     * api when object is SWITCH.
     */

    for (uint32_t index = 0; index < attrCount; ++index)
    {
        auto &attr = attrList[index];

        auto meta = sai_metadata_get_attr_metadata(SAI_OBJECT_TYPE_SWITCH, attr.id);

        if (meta == NULL)
            SWSS_LOG_THROW("failed to find metadata for switch attr %d", attr.id);

        if (meta->attrvaluetype != SAI_ATTR_VALUE_TYPE_POINTER)
            continue;

        switch (attr.id)
        {
            case SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY:
                m_switchNotifications.on_switch_state_change =
                    (sai_switch_state_change_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_SWITCH_ASIC_SDK_HEALTH_EVENT_NOTIFY:
                m_switchNotifications.on_switch_asic_sdk_health_event =
                    (sai_switch_asic_sdk_health_event_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY:
                m_switchNotifications.on_switch_shutdown_request =
                    (sai_switch_shutdown_request_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY:
                m_switchNotifications.on_fdb_event =
                    (sai_fdb_event_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_NAT_EVENT_NOTIFY:
                m_switchNotifications.on_nat_event =
                    (sai_nat_event_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY:
                m_switchNotifications.on_port_state_change =
                    (sai_port_state_change_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY:
                m_switchNotifications.on_packet_event =
                    (sai_packet_event_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_QUEUE_PFC_DEADLOCK_NOTIFY:
                m_switchNotifications.on_queue_pfc_deadlock =
                    (sai_queue_pfc_deadlock_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_BFD_SESSION_STATE_CHANGE_NOTIFY:
                m_switchNotifications.on_bfd_session_state_change =
                    (sai_bfd_session_state_change_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_PORT_HOST_TX_READY_NOTIFY:
                m_switchNotifications.on_port_host_tx_ready =
                    (sai_port_host_tx_ready_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_TWAMP_SESSION_EVENT_NOTIFY:
                m_switchNotifications.on_twamp_session_event =
                    (sai_twamp_session_event_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_SWITCH_MACSEC_POST_STATUS_NOTIFY:
                m_switchNotifications.on_switch_macsec_post_status =
                    (sai_switch_macsec_post_status_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_SWITCH_IPSEC_POST_STATUS_NOTIFY:
                m_switchNotifications.on_switch_ipsec_post_status =
                    (sai_switch_ipsec_post_status_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_MACSEC_POST_STATUS_NOTIFY:
                m_switchNotifications.on_macsec_post_status =
                    (sai_macsec_post_status_notification_fn)attr.value.ptr;
                break;

            case SAI_SWITCH_ATTR_IPSEC_POST_STATUS_NOTIFY:
                m_switchNotifications.on_ipsec_post_status =
                    (sai_ipsec_post_status_notification_fn)attr.value.ptr;
                break;


            default:
                SWSS_LOG_ERROR("pointer for %s is not handled, FIXME!", meta->attridname);
                break;
        }
    }
}

const sai_switch_notifications_t& Switch::getSwitchNotifications() const
{
    SWSS_LOG_ENTER();

    return m_switchNotifications;
}

const std::string& Switch::getHardwareInfo() const
{
    SWSS_LOG_ENTER();

    return m_hardwareInfo;
}
