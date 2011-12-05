  //
  // Use MP Services Protocol to retrieve the number of processors and number of enabled processors
  //
  Status = MpServices->GetNumberOfProcessors (MpServices, &mNumberOfProcessors, &NumberOfEnabledProcessors);

