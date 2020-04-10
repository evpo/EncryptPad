#pragma once
#include <memory>
#include <stdexcept>

namespace LightStateMachine
{
    class TypeErasure
    {
        private:
            struct StorageBase
            {
                virtual ~StorageBase() = default;
            };

            template<class T>
            struct Storage final : public StorageBase
            {
                T* value_;
                Storage(T *value)
                    :value_(value)
                {

                }

            };

            std::unique_ptr<StorageBase> storage_;

        public:
            TypeErasure(const TypeErasure&) = delete;
            TypeErasure(TypeErasure&&) = delete;
            TypeErasure() = default;

            template<class T>
            TypeErasure(T *value):
                storage_(new Storage<T>(value))
            {
            }

            template<class T>
            const TypeErasure& operator=(T *value)
            {
                storage_.reset(new Storage<T>(value));
                return *this;
            }

            template<class T>
            T& Get() const
            {
                auto *p = dynamic_cast<Storage<T>*>(storage_.get());
                if(p == nullptr)
                    throw std::bad_cast();
                return *p->value_;
            }

            template<class T>
            T& Get()
            {
                auto *p = dynamic_cast<Storage<T>*>(storage_.get());
                if(p == nullptr)
                    throw std::bad_cast();
                return *p->value_;
            }
    };
}
