#pragma once

namespace serenity::core
{
    // A 'smart' version of a singleton, which aims at achieving both RAII and a convenient singleton
    // interface. User can own a SingletonInstnace which will handle RAII and follow the
    // singleton pattern throughout the project. That is, somewhere in the code a object that derives SingletonInstance
    // must be created, but from then on anywhere in the code, SingletonInstance::instance() can be used to access the
    // object similar to the singleton pattern.
    // Primary reference :
    // https://github.com/karnkaul/LittleEngineVk/blob/main/engine/include/le/core/mono_instance.hpp.

    template <typename T>
    class SingletonInstance
    {
      protected:
        explicit SingletonInstance()
        {
            if (s_instance)
            {
                throw std::runtime_error(std::format(
                    "Instance already exist for type {}. Did you mean to use instance()?", typeid(T).name()));
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

        static T &instance()
        {
            if (!s_instance)
            {
                throw std::runtime_error(std::format("Instance does not exist for type {}", typeid(T).name()));
            }

            return *s_instance;
        }

      private:
        SingletonInstance(const SingletonInstance &other) = delete;
        SingletonInstance &operator=(const SingletonInstance &other) = delete;

        SingletonInstance(SingletonInstance &&other) = delete;
        SingletonInstance &operator=(SingletonInstance &&other) = delete;

      private:
        static inline T *s_instance{};
    };
} // namespace serenity::core