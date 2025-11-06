#pragma once

#include <QByteArray>
#include <QtGlobal>

#include <algorithm>
#include <functional>
#include <mutex>

namespace dynamicencrypt::core
{
    class ZeroizingBuffer
    {
    public:
        ZeroizingBuffer() = default;
        explicit ZeroizingBuffer(int size) : m_bytes(size, Qt::Uninitialized) {}
        explicit ZeroizingBuffer(QByteArray bytes) : m_bytes(std::move(bytes)) {}

        ZeroizingBuffer(const ZeroizingBuffer &) = delete;
        ZeroizingBuffer &operator=(const ZeroizingBuffer &) = delete;

        ZeroizingBuffer(ZeroizingBuffer &&other) noexcept
            : m_bytes(std::move(other.m_bytes)), m_wiped(other.m_wiped)
        {
            other.m_wiped = true;
        }

        ZeroizingBuffer &operator=(ZeroizingBuffer &&other) noexcept
        {
            if (this != &other)
            {
                secureWipe();
                m_bytes = std::move(other.m_bytes);
                m_wiped = other.m_wiped;
                other.m_wiped = true;
            }
            return *this;
        }

        ~ZeroizingBuffer()
        {
            secureWipe();
        }

        QByteArray &writable() noexcept { return m_bytes; }
        const QByteArray &bytes() const noexcept { return m_bytes; }

        void secureWipe() noexcept
        {
            if (m_wiped)
            {
                return;
            }
            const int size = m_bytes.size();
            volatile unsigned char *ptr = reinterpret_cast<volatile unsigned char *>(m_bytes.data());
            for (int i = 0; i < size; ++i)
            {
                ptr[i] = 0;
            }
            QByteArray proof(size, 0);
            m_bytes.clear();
            m_wiped = true;
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_onWipe)
            {
                s_onWipe(proof);
            }
        }

        static void setOnWipe(std::function<void(const QByteArray &)> callback)
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_onWipe = std::move(callback);
        }

    private:
        QByteArray m_bytes;
        bool m_wiped{false};

        inline static std::mutex s_mutex;
        inline static std::function<void(const QByteArray &)> s_onWipe;
    };

} // namespace dynamicencrypt::core
