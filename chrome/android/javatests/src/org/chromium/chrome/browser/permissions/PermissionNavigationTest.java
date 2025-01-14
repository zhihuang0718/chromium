// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.permissions;

import org.chromium.base.test.util.CallbackHelper;
import org.chromium.base.test.util.CommandLineFlags;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.RetryOnFailure;
import org.chromium.chrome.browser.tab.EmptyTabObserver;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.content.browser.test.util.CriteriaHelper;

/**
 * Test suite for interaction between permissions requests and navigation.
 */
@RetryOnFailure
public class PermissionNavigationTest extends PermissionTestCaseBase {
    private static final String TEST_FILE = "/content/test/data/android/permission_navigation.html";

    public PermissionNavigationTest() {}

    /**
     * Check that modal permission prompts and queued permission requests are removed upon
     * navigation.
     *
     * @throws Exception
     */
    // @MediumTest
    // @Feature({"Permissions"})
    @CommandLineFlags.Add("enable-features=" + MODAL_FLAG)
    // Flaky on official bots, https://crbug.com/699851#c8
    @DisabledTest
    public void testNavigationDismissesModalPermissionPrompt() throws Exception {
        setUpUrl(TEST_FILE);
        runJavaScriptCodeInCurrentTab("requestGeolocationPermission()");
        DialogShownCriteria criteriaShown = new DialogShownCriteria("Dialog not shown", true);
        CriteriaHelper.pollUiThread(criteriaShown);

        runJavaScriptCodeInCurrentTab("navigate()");

        Tab tab = getActivity().getActivityTab();
        final CallbackHelper callbackHelper = new CallbackHelper();
        EmptyTabObserver navigationWaiter = new EmptyTabObserver() {
            @Override
            public void onDidFinishNavigation(Tab tab, String url, boolean isInMainFrame,
                    boolean isErrorPage, boolean hasCommitted, boolean isSameDocument,
                    boolean isFragmentNavigation, Integer pageTransition, int errorCode,
                    int httpStatusCode) {
                callbackHelper.notifyCalled();
            }
        };
        tab.addObserver(navigationWaiter);
        callbackHelper.waitForCallback(0);
        tab.removeObserver(navigationWaiter);

        DialogShownCriteria criteriaNotShown = new DialogShownCriteria("Dialog shown", false);
        CriteriaHelper.pollUiThread(criteriaNotShown);
    }
}
