namespace fw {

class Framework {
   public:
    Framework();
    Framework(const Framework&) = delete;
    Framework(Framework&&) = delete;
    Framework& operator=(const Framework&) = delete;
    Framework& operator=(Framework&&) = delete;

    void callFramework() const;

   private:
};

}  // namespace fw