LegacyAnimatorAdapter(const AnimationBuilder& abuilder,
                          const skjson::ObjectValue* jprop,
                          std::function<void(const T&)>&& execute)
        : fApplyFunc(std::move(execute)) {
        if (!this->bind<T>(abuilder, jprop, &fValue)) {
            fValue = T();
        }
    }