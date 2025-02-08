LegacyAnimatorAdapter(const AnimationBuilder& abuilder,
                          const skjson::ObjectValue* jprop,
                          std::function<void(const T&)>&& apply)
        : fApplyFunc(std::move(apply)) {
        if (!this->bind<T>(abuilder, jprop, &fValue)) {
            fValue = T();
        }
    }