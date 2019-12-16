    // DER BEHINDERTE WAL (der erste)
    /*
    segments.push_back(Segment(85.33, 28.33, 81.25, 39.26));
    segments.push_back(Segment(81.08, 41.08, 84.17, 55.76));
    segments.push_back(Segment(83.33, 59.17, 80.10, 58.37));
    segments.push_back(Segment(77.17, 57.08, 71.74, 48.68));
    segments.push_back(Segment(70.58, 47.75, 67.34, 46.99));
    segments.push_back(Segment(64.58, 47.42, 47.93, 58.49));
    segments.push_back(Segment(46.08, 59.75, 34.58, 61.71));
    segments.push_back(Segment(30.83, 61.67, 16.56, 57.06));
    segments.push_back(Segment(13.83, 55.42, 8.85, 44.87));
    segments.push_back(Segment(8.83, 42.58, 9.03, 34.25));
    segments.push_back(Segment(9.58, 30.17, 19.75, 19.14));
    segments.push_back(Segment(22.17, 17.67, 30.35, 16.11));
    segments.push_back(Segment(32.50, 16.00, 47.50, 15.93));
    segments.push_back(Segment(51.50, 17.08, 61.28, 26.14));
    segments.push_back(Segment(62.33, 28.42, 63.82, 33.19));
    segments.push_back(Segment(65.17, 36.08, 68.89, 39.42));
    segments.push_back(Segment(71.50, 41.17, 74.82, 40.84));
    segments.push_back(Segment(77.50, 40.00, 82.71, 31.46));
    segments.push_back(Segment(14.58, 51.83, 36.12, 49.46));
    segments.push_back(Segment(39.00, 49.08, 47.15, 47.36));
    segments.push_back(Segment(21.42, 51.92, 24.30, 56.00));
    segments.push_back(Segment(25.67, 55.75, 26.92, 52.66));
    segments.push_back(Segment(28.83, 27.58, 27.29, 26.95));
    segments.push_back(Segment(25.33, 26.75, 24.01, 27.76));
    segments.push_back(Segment(38.50, 13.92, 37.25, 10.83));
    segments.push_back(Segment(34.92, 12.92, 35.51, 11.36));
    segments.push_back(Segment(39.67, 12.50, 39.79, 10.84));
    segments.push_back(Segment(41.67, 8.42, 43.29, 8.05));
    segments.push_back(Segment(46.17, 10.08, 46.17, 10.08));
    segments.push_back(Segment(32.67, 9.08, 32.67, 9.08));
    segments.push_back(Segment(30.75, 13.00, 30.75, 13.00));
    segments.push_back(Segment(38.08, 8.17, 38.08, 8.17));
    // */

    // QM IS OK
    /*
    segments.push_back(Segment(14.1667, 7.33333, 6.25, 9.25));
    segments.push_back(Segment(5.75, 10.3333, 3.66667, 14.4167));
    segments.push_back(Segment(4.16667, 15.75, 5.91667, 19.6667));
    segments.push_back(Segment(7.16667, 20.4167, 10.6667, 22));
    segments.push_back(Segment(12.5, 22.25, 17.6667, 19.25));
    segments.push_back(Segment(18.3333, 16.9167, 17, 8.91667));
    segments.push_back(Segment(13.5833, 18.5833, 17.6667, 25.6667));
    segments.push_back(Segment(24.3333, 24.5833, 33.1667, 8.75));
    segments.push_back(Segment(33.5833, 11.75, 35.75, 25.1667));
    segments.push_back(Segment(36.9167, 24.8333, 46, 8));
    segments.push_back(Segment(48, 8.41667, 52.1667, 32.25));
    segments.push_back(Segment(12.75, 33.6667, 14.5, 49.8333));
    segments.push_back(Segment(22.5833, 31.9167, 22.25, 32.1667));
    segments.push_back(Segment(28.4167, 32.3333, 22.5, 32.1667));
    segments.push_back(Segment(21.6667, 32.9167, 19.3333, 38));
    segments.push_back(Segment(20.5833, 38.4167, 27.6667, 40.8333));
    segments.push_back(Segment(26.75, 40.9167, 27.5833, 43.8333));
    segments.push_back(Segment(26.5833, 44.75, 23.3333, 47.5833));
    segments.push_back(Segment(21.75, 47.3333, 18.3333, 45.8333));
    segments.push_back(Segment(41.1667, 45.1667, 37.0833, 46.5));
    segments.push_back(Segment(36, 47.0833, 32.9167, 54.25));
    segments.push_back(Segment(33.4167, 55.3333, 39.4167, 59.5833));
    segments.push_back(Segment(41.3333, 59.5833, 47.9167, 57.9167));
    segments.push_back(Segment(48.8333, 56.5833, 50.1667, 52.1667));
    segments.push_back(Segment(49.3333, 50.5, 44.3333, 45.5833));
    segments.push_back(Segment(52.0833, 60.0833, 55.4167, 43.25));
    segments.push_back(Segment(55.1667, 52.3333, 67.5, 42.4167));
    segments.push_back(Segment(56.6667, 52.25, 62.5, 59.3333));
    // */
    
    // frame
    /*
    segments.push_back(Segment(0, 0, A, 0));
    segments.push_back(Segment(0, B, A, B));
    segments.push_back(Segment(0, D, 0, B));
    segments.push_back(Segment(B, D, B, B));
    // */
    
    // full matrix
    /*
    for(float row = 0; row < A; row += D)
    {
        segments.push_back(Segment(0, row, A, row));
    }
    // */