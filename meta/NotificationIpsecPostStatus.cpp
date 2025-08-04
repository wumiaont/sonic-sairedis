#include "NotificationIpsecPostStatus.h"

#include "swss/logger.h"

#include "sai_serialize.h"

using namespace sairedis;

NotificationIpsecPostStatus::NotificationIpsecPostStatus(
        _In_ const std::string& serializedNotification):
    Notification(
            SAI_SWITCH_NOTIFICATION_TYPE_IPSEC_POST_STATUS,
            serializedNotification)
{
    SWSS_LOG_ENTER();

    sai_deserialize_ipsec_post_status_ntf(serializedNotification, m_switchId, m_ipsec_post_status);
}

sai_object_id_t NotificationIpsecPostStatus::getSwitchId() const
{
    SWSS_LOG_ENTER();

    return m_switchId;
}

sai_object_id_t NotificationIpsecPostStatus::getAnyObjectId() const
{
    SWSS_LOG_ENTER();

    return m_switchId;
}

void NotificationIpsecPostStatus::processMetadata(
        _In_ std::shared_ptr<saimeta::Meta> meta) const
{
    SWSS_LOG_ENTER();

    meta->meta_sai_on_ipsec_post_status(m_switchId, m_ipsec_post_status);
}

void NotificationIpsecPostStatus::executeCallback(
        _In_ const sai_switch_notifications_t& switchNotifications) const
{
    SWSS_LOG_ENTER();

    if (switchNotifications.on_ipsec_post_status)
    {
        switchNotifications.on_ipsec_post_status(m_switchId, m_ipsec_post_status);
    }
}
