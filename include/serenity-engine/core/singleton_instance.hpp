#pragma once

namespace serenity::core
{
    // A 'smart' version of a singleton, which aims at achieving both RAII and a convenient singleton
    // interface. User can own a SingletonInstnace which will handle RAII and follow the
    // singleton pattern throughout the project.
    // Primary reference :
    // https://github.com/karnkaul/LittleEngineVk/blob/main/engine/include/le/core/mono_instance.hpp.

    template <typename T> class SingletonInstance
    {
      protected:
        explicit SingletonInstance()
        {
            if (s_instance)
            {
                throw std::runtime_error(std::format("Instance already exist for type {}", typeid(T).name()));
            }

            s_instance = static_cast<T *>(this);
        }

        virtual ~SingletonInstance()
        {
            s_instance = nullptr;
        }

      public:
        static bool exists()
        {
            return s_instance != nullptr;
        }

        static T &get()
        {
            if (!s_instance)
            {
                throw std::runtime_error(std::format("Instance does not exist for type {}", typeid(T).name()));
            }

            return *s_instance;
        }

      protected:
        SingletonInstance(const SingletonInstance &other) = delete;
        SingletonInstance &operator=(const SingletonInstance &other) = delete;

        SingletonInstance(SingletonInstance &&other) = delete;
        SingletonInstance &operator=(SingletonInstance &&other) = delete;

      protected:
        static inline T *s_instance{};
    };
} // namespace serenity::core