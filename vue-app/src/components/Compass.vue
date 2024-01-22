<template>
  <div class="compass border-box flex-vst gap-one">
    <div id="compass-svg">
      <!-- Corner Labels -->
      <!-- ============= -->

      <!-- Rotation -->
      <div
        v-if="rotorStore.rotor.rotation === -1"
        class="rotation-lbl corner-lbl bold medium no-select"
      >
        <div class="rotate-ccw">
          <Icon icon="fa-solid fa-rotate-left"></Icon>
        </div>
      </div>

      <div
        v-if="rotorStore.rotor.rotation === 0"
        class="rotation-lbl corner-lbl bold medium no-select"
      >
        STOP
      </div>

      <div
        v-if="rotorStore.rotor.rotation === 1"
        class="rotation-lbl corner-lbl bold medium no-select"
      >
        <div class="rotate-cw">
          <Icon icon="fa-solid fa-rotate-right"></Icon>
        </div>
      </div>

      <!-- Overlap -->
      <div v-if="rotorStore.isOverlap" id="overlap-lbl" class="corner-lbl medium bold no-select">
        OL
      </div>

      <!-- Lock-->
      <div
        v-if="settingsStore.lock.isLocked"
        id="lock-lbl"
        class="corner-lbl medium bold no-select"
      >
        <Icon icon="fa-solid fa-lock"></Icon>
      </div>

      <!-- Compass SVG -->
      <!-- =========== -->
      <svg
        id="compass"
        ref="compass"
        @click="requestAngle"
        :class="{
          'mouse-inside': uiStore.ui.isMouseInCompass && !settingsStore.isLockedByElse,
          locked: settingsStore.isLockedByElse
        }"
        viewBox="0 30 1000 940"
        style="fill-rule: evenodd; clip-rule: evenodd; stroke-linejoin: round; stroke-miterlimit: 2"
      >
        <!-- Basic Shapes -->
        <!-- ------------ -->
        <g>
          <circle
            id="cmp-background"
            cx="500"
            cy="500"
            r="410"
            style="fill: var(--compass-background-color)"
          />

          <path
            id="cmp-cross"
            style="fill: var(--compass-cross-color)"
            d="M501.517,234.571C500.506,234.565 499.494,234.565 498.483,234.571L498.483,401.327C445.169,402.147 402.147,445.169 401.327,498.483L234.571,498.483C234.565,499.494 234.565,500.506 234.571,501.517L401.327,501.517C402.147,554.831 445.169,597.853 498.483,598.673L498.483,765.429C499.494,765.435 500.506,765.435 501.517,765.429L501.517,598.673C554.831,597.853 597.853,554.831 598.673,501.517L765.429,501.517C765.435,500.506 765.435,499.494 765.429,498.483L598.673,498.483C597.853,445.169 554.831,402.147 501.517,401.327L501.517,234.571ZM500,404.916C552.478,404.916 595.084,447.522 595.084,500C595.084,552.478 552.478,595.084 500,595.084C447.522,595.084 404.916,552.478 404.916,500C404.916,447.522 447.522,404.916 500,404.916Z"
          />

          <path
            id="cmp-ring"
            :style="ringColor"
            d="M510.391,107.886C574.233,109.545 634.3,126.494 687.078,155.217L702.351,149.518L705.058,165.628C757.644,197.968 802.037,242.361 834.372,294.942L850.482,297.649L844.783,312.922C873.499,365.696 890.45,425.765 892.114,489.609L904.701,500L892.114,510.391C890.455,574.233 873.506,634.3 844.783,687.078L850.482,702.351L834.372,705.058C802.032,757.644 757.639,802.037 705.058,834.372L702.351,850.482L687.078,844.783C634.304,873.499 574.235,890.45 510.391,892.114L500,904.701L489.609,892.114C425.767,890.455 365.7,873.506 312.922,844.783L297.649,850.482L294.942,834.372C242.356,802.032 197.963,757.639 165.628,705.058L149.518,702.351L155.217,687.078C126.501,634.304 109.55,574.235 107.886,510.391L95.299,500L107.886,489.609C109.545,425.767 126.494,365.7 155.217,312.922L149.518,297.649L165.628,294.942C197.968,242.356 242.361,197.963 294.942,165.628L297.649,149.518L312.922,155.217C365.696,126.501 425.765,109.55 489.609,107.886L500,95.299L510.391,107.886ZM498.446,878.882C499.705,878.888 500.963,878.888 502.221,878.882L502.221,839.034C500.963,839.041 499.705,839.041 498.446,839.034L498.446,878.882ZM506.223,135.849C506.072,145.807 506.081,155.765 506.252,165.722C502.084,165.644 497.916,165.644 493.748,165.722C493.919,155.765 493.928,145.807 493.777,135.849C485.654,135.984 477.595,136.387 469.587,137.05L470.321,145.807C469.455,145.88 468.589,145.955 467.723,146.034L466.925,137.281C457.295,138.15 447.773,139.395 438.377,140.998L441.556,158.982C440.465,159.169 439.374,159.362 438.284,159.559L435.12,141.569C425.611,143.283 416.229,145.366 407.01,147.796L409.267,156.342C408.426,156.564 407.587,156.789 406.748,157.017L404.429,148.487C395.115,151.016 385.949,153.907 376.967,157.135L383.227,174.312C382.184,174.685 381.144,175.064 380.105,175.448L373.86,158.267C364.83,161.606 355.967,165.293 347.274,169.317L350.97,177.318C350.18,177.683 349.393,178.05 348.606,178.42L344.854,170.449C337.035,174.14 329.369,178.101 321.86,182.324C325.678,188.984 329.523,195.629 333.393,202.26C330.726,203.752 328.079,205.281 325.453,206.844C321.648,200.18 317.816,193.53 313.96,186.898C306.537,191.32 299.285,195.999 292.215,200.923L297.201,208.099C296.487,208.595 295.775,209.094 295.065,209.595L290.028,202.458C282.183,208.008 274.566,213.86 267.202,219.99L278.877,233.892C278.025,234.6 277.177,235.311 276.332,236.027L264.67,222.117C257.347,228.327 250.273,234.82 243.466,241.577L249.594,247.75C248.977,248.362 248.362,248.977 247.75,249.594L241.577,243.466C234.817,250.275 228.325,257.349 222.117,264.67L236.027,276.332C235.311,277.177 234.6,278.025 233.892,278.877L219.99,267.202C213.857,274.568 208.008,282.18 202.458,290.028L209.595,295.065C209.094,295.775 208.595,296.487 208.099,297.201L200.923,292.215C196.001,299.283 191.323,306.533 186.898,313.96C193.53,317.816 200.18,321.648 206.844,325.453C205.281,328.079 203.752,330.726 202.26,333.393C195.629,329.523 188.984,325.678 182.324,321.86C178.105,329.362 174.144,337.029 170.449,344.854L178.42,348.606C178.05,349.393 177.683,350.18 177.318,350.97L169.317,347.274C165.297,355.954 161.609,364.819 158.267,373.86L175.448,380.105C175.064,381.144 174.685,382.184 174.312,383.227L157.135,376.967C153.902,385.962 151.014,395.122 148.487,404.429L157.017,406.748C156.789,407.587 156.564,408.426 156.342,409.267L147.796,407.01C145.361,416.242 143.279,425.619 141.569,435.12L159.559,438.284C159.362,439.374 159.169,440.465 158.982,441.556L140.998,438.377C139.393,447.773 138.149,457.293 137.281,466.925L146.034,467.723C145.955,468.589 145.88,469.455 145.807,470.321L137.05,469.587C136.388,477.594 135.985,485.656 135.849,493.777C145.807,493.928 155.765,493.919 165.722,493.748C165.644,497.916 165.644,502.084 165.722,506.252C155.765,506.081 145.807,506.072 135.849,506.223C135.984,514.346 136.387,522.405 137.05,530.413L145.807,529.679C145.88,530.545 145.955,531.411 146.034,532.277L137.281,533.075C138.15,542.705 139.395,552.227 140.998,561.623L158.982,558.444C159.169,559.535 159.362,560.626 159.559,561.716L141.569,564.88C143.283,574.389 145.366,583.771 147.796,592.99L156.342,590.733C156.564,591.574 156.789,592.413 157.017,593.252L148.487,595.571C151.016,604.885 153.907,614.051 157.135,623.033L174.312,616.773C174.685,617.816 175.064,618.856 175.448,619.895L158.267,626.14C161.606,635.17 165.293,644.033 169.317,652.726L177.318,649.03C177.683,649.82 178.05,650.607 178.42,651.394L170.449,655.146C174.14,662.965 178.101,670.631 182.324,678.14C188.984,674.322 195.629,670.477 202.26,666.607C203.752,669.274 205.281,671.921 206.844,674.547C200.18,678.352 193.53,682.184 186.898,686.04C191.32,693.463 195.999,700.715 200.923,707.785L208.099,702.799C208.595,703.513 209.094,704.225 209.595,704.935L202.458,709.972C208.008,717.817 213.86,725.434 219.99,732.798L233.892,721.123C234.6,721.975 235.311,722.823 236.027,723.668L222.117,735.33C228.327,742.653 234.82,749.727 241.577,756.534L247.75,750.406C248.362,751.023 248.977,751.638 249.594,752.25L243.466,758.423C250.275,765.183 257.349,771.675 264.67,777.883L276.332,763.973C277.177,764.689 278.025,765.4 278.877,766.108L267.202,780.01C274.568,786.143 282.18,791.992 290.028,797.542L295.065,790.405C295.775,790.906 296.487,791.405 297.201,791.901L292.215,799.077C299.283,803.999 306.533,808.677 313.96,813.102C317.816,806.47 321.648,799.82 325.453,793.156C328.079,794.719 330.726,796.248 333.393,797.74C329.523,804.371 325.678,811.016 321.86,817.676C329.362,821.895 337.029,825.856 344.854,829.551L348.606,821.58C349.393,821.95 350.18,822.317 350.97,822.682L347.274,830.683C355.954,834.703 364.819,838.391 373.86,841.733L380.105,824.552C381.144,824.936 382.184,825.315 383.227,825.688L376.967,842.865C385.962,846.098 395.122,848.986 404.429,851.513L406.748,842.983C407.587,843.211 408.426,843.436 409.267,843.658L407.01,852.204C416.242,854.639 425.619,856.721 435.12,858.431L438.284,840.441C439.374,840.638 440.465,840.831 441.556,841.018L438.377,859.002C447.773,860.607 457.293,861.851 466.925,862.719L467.723,853.966C468.589,854.045 469.455,854.12 470.321,854.193L469.587,862.95C477.594,863.612 485.656,864.015 493.777,864.151C493.928,854.193 493.919,844.235 493.748,834.278C497.916,834.356 502.084,834.356 506.252,834.278C506.081,844.235 506.072,854.193 506.223,864.151C514.346,864.016 522.405,863.613 530.413,862.95L529.679,854.193C530.545,854.12 531.411,854.045 532.277,853.966L533.075,862.719C542.705,861.85 552.227,860.605 561.623,859.002L558.444,841.018C559.535,840.831 560.626,840.638 561.716,840.441L564.88,858.431C574.389,856.717 583.771,854.634 592.99,852.204L590.733,843.658C591.574,843.436 592.413,843.211 593.252,842.983L595.571,851.513C604.885,848.984 614.051,846.093 623.033,842.865L616.773,825.688C617.816,825.315 618.856,824.936 619.895,824.552L626.14,841.733C635.17,838.394 644.033,834.707 652.726,830.683L649.03,822.682C649.82,822.317 650.607,821.95 651.394,821.58L655.146,829.551C662.965,825.86 670.631,821.899 678.14,817.676C674.322,811.016 670.477,804.371 666.607,797.74C669.274,796.248 671.921,794.719 674.547,793.156C678.352,799.82 682.184,806.47 686.04,813.102C693.463,808.68 700.715,804.001 707.785,799.077L702.799,791.901C703.513,791.405 704.225,790.906 704.935,790.405L709.972,797.542C717.817,791.992 725.434,786.14 732.798,780.01L721.123,766.108C721.975,765.4 722.823,764.689 723.668,763.973L735.33,777.883C742.653,771.673 749.727,765.18 756.534,758.423L750.406,752.25C751.023,751.638 751.638,751.023 752.25,750.406L758.423,756.534C765.183,749.725 771.675,742.651 777.883,735.33L763.973,723.668C764.689,722.823 765.4,721.975 766.108,721.123L780.01,732.798C786.143,725.432 791.992,717.82 797.542,709.972L790.405,704.935C790.906,704.225 791.405,703.513 791.901,702.799L799.077,707.785C803.999,700.717 808.677,693.467 813.102,686.04C806.47,682.184 799.82,678.352 793.156,674.547C794.719,671.921 796.248,669.274 797.74,666.607C804.371,670.477 811.016,674.322 817.676,678.14C821.895,670.638 825.856,662.971 829.551,655.146L821.58,651.394C821.95,650.607 822.317,649.82 822.682,649.03L830.683,652.726C834.703,644.046 838.391,635.181 841.733,626.14L824.552,619.895C824.936,618.856 825.315,617.816 825.688,616.773L842.865,623.033C846.098,614.038 848.986,604.878 851.513,595.571L842.983,593.252C843.211,592.413 843.436,591.574 843.658,590.733L852.204,592.99C854.639,583.758 856.721,574.381 858.431,564.88L840.441,561.716C840.638,560.626 840.831,559.535 841.018,558.444L859.002,561.623C860.607,552.227 861.851,542.707 862.719,533.075L853.966,532.277C854.045,531.411 854.12,530.545 854.193,529.679L862.95,530.413C863.612,522.406 864.015,514.344 864.151,506.223C854.193,506.072 844.235,506.081 834.278,506.252C834.356,502.084 834.356,497.916 834.278,493.748C844.235,493.919 854.193,493.928 864.151,493.777C864.016,485.654 863.613,477.595 862.95,469.587L854.193,470.321C854.12,469.455 854.045,468.589 853.966,467.723L862.719,466.925C861.85,457.295 860.605,447.773 859.002,438.377L841.018,441.556C840.831,440.465 840.638,439.374 840.441,438.284L858.431,435.12C856.717,425.611 854.634,416.229 852.204,407.01L843.658,409.267C843.436,408.426 843.211,407.587 842.983,406.748L851.513,404.429C848.984,395.115 846.093,385.949 842.865,376.967L825.688,383.227C825.315,382.184 824.936,381.144 824.552,380.105L841.733,373.86C838.394,364.83 834.707,355.967 830.683,347.274L822.682,350.97C822.317,350.18 821.95,349.393 821.58,348.606L829.551,344.854C825.86,337.035 821.899,329.369 817.676,321.86C811.016,325.678 804.371,329.523 797.74,333.393C796.248,330.726 794.719,328.079 793.156,325.453C799.82,321.648 806.47,317.816 813.102,313.96C808.68,306.537 804.001,299.285 799.077,292.215L791.901,297.201C791.405,296.487 790.906,295.775 790.405,295.065L797.542,290.028C791.992,282.183 786.14,274.566 780.01,267.202L766.108,278.877C765.4,278.025 764.689,277.177 763.973,276.332L777.883,264.67C771.673,257.347 765.18,250.273 758.423,243.466L752.25,249.594C751.638,248.977 751.023,248.362 750.406,247.75L756.534,241.577C749.725,234.817 742.651,228.325 735.33,222.117L723.668,236.027C722.823,235.311 721.975,234.6 721.123,233.892L732.798,219.99C725.432,213.857 717.82,208.008 709.972,202.458L704.935,209.595C704.225,209.094 703.513,208.595 702.799,208.099L707.785,200.923C700.717,196.001 693.467,191.323 686.04,186.898C682.184,193.53 678.352,200.18 674.547,206.844C671.921,205.281 669.274,203.752 666.607,202.26C670.477,195.629 674.322,188.984 678.14,182.324C670.638,178.105 662.971,174.144 655.146,170.449L651.394,178.42C650.607,178.05 649.82,177.683 649.03,177.318L652.726,169.317C644.046,165.297 635.181,161.609 626.14,158.267L619.895,175.448C618.856,175.064 617.816,174.685 616.773,174.312L623.033,157.135C614.038,153.902 604.878,151.014 595.571,148.487L593.252,157.017C592.413,156.789 591.574,156.564 590.733,156.342L592.99,147.796C583.758,145.361 574.381,143.279 564.88,141.569L561.716,159.559C560.626,159.362 559.535,159.169 558.444,158.982L561.623,140.998C552.227,139.393 542.707,138.149 533.075,137.281L532.277,146.034C531.411,145.955 530.545,145.88 529.679,145.807L530.413,137.05C522.406,136.388 514.344,135.985 506.223,135.849ZM878.882,501.889C878.888,500.63 878.888,499.37 878.882,498.111L839.069,498.111C839.076,499.37 839.076,500.63 839.069,501.889L878.882,501.889ZM121.786,498.111C121.78,499.37 121.78,500.63 121.786,501.889L161.598,501.889C161.591,500.63 161.591,499.37 161.598,498.111L121.786,498.111ZM502.221,121.118C500.963,121.112 499.705,121.112 498.446,121.118L498.446,160.966C499.705,160.959 500.963,160.959 502.221,160.966L502.221,121.118Z"
          />

          <path
            id="cmp-ol-indicator"
            :style="
              settingsStore.isLockedByElse
                ? 'fill: var(--compass-color)'
                : 'fill: var(--alert-color)'
            "
            :class="{ 'hide-opacity': !rotorStore.isOverlap }"
            d="M878.214,493.398C879.363,493.378 880.457,492.9 881.251,492.07C882.046,491.24 882.476,490.127 882.446,488.978C876.646,286.361 713.639,123.354 511.022,117.557C509.874,117.527 508.762,117.957 507.932,118.751C507.102,119.545 506.624,120.637 506.604,121.786C506.602,121.787 506.602,121.788 506.602,121.789C506.561,124.143 508.425,126.091 510.778,126.152C708.84,131.818 868.182,291.16 873.85,489.222C873.912,491.574 875.858,493.437 878.211,493.396C878.212,493.398 878.213,493.398 878.214,493.398Z"
          />
        </g>

        <!-- Favorites Dots -->
        <!-- -------------- -->
        <g v-if="uiStore.ui.hasFavoritesDots">
          <circle
            id="cmp-favorites-ring"
            cx="500"
            cy="500"
            :r="favoritesRingRadius + 2"
            :stroke="favoritesRingColor"
            stroke-width="4"
            fill="none"
          />

          <g
            v-for="fav in settingsStore.favorites.array"
            :key="fav.id"
            class="cmp-favorite-dot"
            :class="{ 'cmp-lbl-disabled': settingsStore.isLockedByElse }"
            @click="
              if (!settingsStore.isLockedByElse) {
                umbrellaStore.sendTarget(fav.angle);
              }
            "
          >
            <title>{{ fav.name }}</title>
            <circle
              :cx="favoriteXCoordinate(fav.angle)"
              :cy="favoriteYCoordinate(fav.angle)"
              r="20"
              style="fill: var(--accent-color)"
            />
            <text
              class="no-select"
              :x="favoriteXCoordinate(fav.angle)"
              :y="favoriteYCoordinate(fav.angle) + 2"
              style="fill: var(--text-color-accent)"
              dominant-baseline="middle"
            >
              {{ fav.id }}
            </text>
          </g>
        </g>

        <!-- Cardinals Labels -->
        <!-- ---------------- -->
        <g
          v-if="uiStore.ui.hasCardinalLabels"
          id="cmp-cardinals"
          class="small bold no-select"
          style="fill: var(--compass-cardinals-color)"
        >
          <text
            v-for="cardinal in cardinalLabels"
            :x="cardinal.x"
            :y="cardinal.y"
            :transform="'rotate(' + cardinal.rot + ')'"
            :style="'transform-origin:' + cardinal.x + 'px ' + cardinal.y + 'px'"
          >
            {{ cardinal.dir }}
          </text>
        </g>

        <!-- Needles-->
        <!-- ------- -->
        <g id="cmp-needle" :style="{ transform: 'rotate(' + rotorStore.angle1D + 'deg)' }">
          <path :style="ringColor" d="M500,170 L 545,500 L 500,580 L 455,500 L 500,170Z" />
        </g>

        <g
          id="cmp-req-needle"
          style="fill: var(--compass-req-needle-color)"
          :style="{ transform: 'rotate(' + uiStore.ui.requestAngle + 'deg)' }"
          :class="{ 'hide-opacity': !uiStore.ui.isMouseInCompass || settingsStore.isLockedByElse }"
        >
          <path d="M500,120 L 515,160 L 515,700 L 500,700 L 485,700 L 485,160" />
        </g>

        <g
          id="cmp-target-needle"
          v-if="rotorStore.hasTarget"
          :style="{ transform: 'rotate(' + rotorStore.rotor.target + 'deg)' }"
        >
          <path :style="ringColor" d="M500,170 L 545,500 L 500,580 L 455,500 L 500,170Z" />
        </g>

        <!-- Center Circle -->
        <!-- ------------- -->
        <g id="cmp-center" class="no-select">
          <circle cx="500" cy="500" r="90" style="fill: var(--compass-center-color)" />

          <text
            x="500px"
            y="452px"
            style="fill: var(--text-color-darker)"
            dominant-baseline="middle"
          >
            {{ rotorStore.cardinal }}
          </text>

          <text
            class="bold"
            x="500px"
            y="500px"
            style="fill: var(--text-color)"
            dominant-baseline="middle"
          >
            {{ rotorStore.angle }}°
          </text>

          <text
            :class="{
              'hide-opacity': !uiStore.ui.isMouseInCompass || settingsStore.isLockedByElse
            }"
            x="500px"
            y="548"
            style="fill: var(--compass-degrees-color)"
            dominant-baseline="middle"
          >
            {{ uiStore.ui.requestAngle.toFixed(1) }}°
          </text>
        </g>

        <!-- Degree Labels -->
        <g
          v-if="uiStore.ui.hasDegreeLabels"
          id="cmp-degrees"
          class="no-select"
          style="fill: var(--compass-degrees-color)"
        >
          <text
            v-for="deg in degreeLabels"
            :x="deg.x"
            :y="deg.y"
            @click="
              if (!settingsStore.isLockedByElse) {
                umbrellaStore.sendTarget(deg.angle);
              }
            "
            @keydown.enter="
              if (!settingsStore.isLockedByElse) {
                umbrellaStore.sendTarget(deg.angle);
              }
            "
            @keyup.space="
              if (!settingsStore.isLockedByElse) {
                umbrellaStore.sendTarget(deg.angle);
              }
            "
            @keydown.space.prevent=""
            :class="{ 'cmp-lbl-disabled': settingsStore.isLockedByElse }"
            tabindex="0"
            dominant-baseline="middle"
          >
            {{ deg.angle }}°
          </text>
        </g>
      </svg>

      <!-- ****************************** -->

      <!--svg
        id="compass"
        ref="compass"
        :class="{
          'mouse-inside': uiStore.ui.isMouseInCompass && !settingsStore.isLockedByElse,
          locked: settingsStore.isLockedByElse
        }"
        viewBox="0 0 1000 1000"
        style="fill-rule: evenodd"
        @click="requestAngle"
      >
        <path
          d="M500,0C775.958,0 1000,224.042 1000,500C1000,775.958 775.958,1000 500,1000C224.042,1000 0,775.958 0,500C0,224.042 224.042,0 500,0ZM500,50C748.362,50 950,251.638 950,500C950,748.362 748.362,950 500,950C251.638,950 50,748.362 50,500C50,251.638 251.638,50 500,50Z"
          :style="ringColor"
        />
        <g
          id="cmp-req-needle"
          :style="{ transform: 'rotate(' + uiStore.ui.requestAngle + 'deg)' }"
          :class="{ hidden: !uiStore.ui.isMouseInCompass || settingsStore.isLockedByElse }"
        >
          <path d="M500,15L515,40L515,700L485,700L485,40L" />
        </g>
        <g
          v-if="rotorStore.hasTarget"
          id="cmp-target-needle"
          :style="{ transform: 'rotate(' + rotorStore.rotor.target + 'deg)' }"
        >
          <path d="M500,200L550,500L500,650L450,500L500,200" :style="ringColor" />
          >
        </g>
        <g id="cmp-needle" :style="{ transform: 'rotate(' + rotorStore.angle1D + 'deg)' }">
          <path d="M500,200L550,500L500,650L450,500L500,200" :style="ringColor" />
        </g>
      </svg!-->
    </div>

    <!-- Additional Information -->
    <!-- ====================== -->
    <div class="compass-label border-box">
      <span class="small txt-dark">Position</span>
      <span class="small txt-dark">Zielposition</span>
      <span class="small txt-dark">Uhrzeit</span>
      <span class="large bold">{{ rotorStore.angle1D }}°</span>
      <span class="large bold">{{ target }}</span>
      <span class="medium bold monospace" title="Die aktuelle Systemzeit">{{ currentTime }}</span>
    </div>
  </div>
</template>

<script setup>
import { ref, reactive, computed, watch, onMounted, onUnmounted } from 'vue';
import { useEventListener, useMouseInElement } from '@vueuse/core';

import { useUmbrellaStore } from '@/stores/umbrella';
import { useRotorStore } from '@/stores/rotor';
import { useSettingsStore } from '@/stores/settings';
import { useUIStore } from '@/stores/ui';

const umbrellaStore = useUmbrellaStore();
const rotorStore = useRotorStore();
const settingsStore = useSettingsStore();
const uiStore = useUIStore();

const compass = ref(null);

const degreeLabels = [
  { angle: 0, x: 508, y: 68 },
  { angle: 30, x: 732, y: 122 },
  { angle: 60, x: 900, y: 295 },
  { angle: 90, x: 954, y: 503 },
  { angle: 120, x: 905, y: 722 },
  { angle: 150, x: 738, y: 888 },
  { angle: 180, x: 511, y: 940 },
  { angle: 210, x: 268, y: 888 },
  { angle: 240, x: 93, y: 722 },
  { angle: 270, x: 42, y: 503 },
  { angle: 300, x: 91, y: 295 },
  { angle: 330, x: 270, y: 122 }
];

const cardinalLabels = [
  { dir: 'N', x: 486, y: 214, rot: 0 },
  { dir: 'NO', x: 690, y: 265, rot: 45 },
  { dir: 'O', x: 786, y: 513, rot: 0 },
  { dir: 'SO', x: 715, y: 757, rot: -45 },
  { dir: 'S', x: 487, y: 815, rot: -0 },
  { dir: 'SW', x: 243, y: 712, rot: 45 },
  { dir: 'W', x: 182, y: 513, rot: 0 },
  { dir: 'NW', x: 263, y: 313, rot: -45 }
];

// Target position, null replaced with dashes
const target = computed(() => {
  if (rotorStore.rotor.target == null) {
    return '--';
  }
  return String(rotorStore.rotor.target) + '°';
});

// Current time label
// ------------------
const today = ref(new Date());
const currentTime = computed(() => {
  let time = String(today.value.getHours()).padStart(2, '0') + ":";
  time += String(today.value.getMinutes()).padStart(2, '0') + ":";
  time += String(today.value.getSeconds()).padStart(2, '0');
  return time;
});

// Enable Update time with mount
let intervalId;
onMounted(() => {
  intervalId = setInterval(() => {
    today.value = new Date();
  }, 100)
})

// Disable update time on unmount
onUnmounted(() => {
  clearInterval(intervalId)
});

// Change ring color if rotor is locked by someone else
// ----------------------------------------------------
const ringColor = computed(() => {
  if (settingsStore.isLockedByElse) {
    return 'fill: var(--alert-color)';
  } else {
    return 'fill: var(--compass-color)';
  }
});

// Get and set request angle from mouse position in compass
// --------------------------------------------------------
function setRequestAngle(event) {
  // Transform mouse position from insde compass
  // bounding-rectangle to polar coordinates.
  const rect = compass.value.getBoundingClientRect();
  const x = event.offsetX - rect.width / 2;
  const y = (event.offsetY - rect.height / 2) * -1;
  const radius = Math.sqrt(x * x + y * y);

  // Check wether mouse is outside of compass circle
  if (radius < rect.width * (0.5 - 0.09)) {
    uiStore.ui.isMouseInCompass = true;
    let angle = (Math.atan2(x, y) * 180) / Math.PI;
    if (angle < 0) {
      angle = 360 + angle;
    }
    uiStore.ui.requestAngle = angle;
  } else {
    uiStore.ui.isMouseInCompass = false;
  }
}

// Register event listener
useEventListener(compass, 'mousemove', setRequestAngle);

// Watch when mouse moves outside compass element to hide request needle
const mouse = reactive(useMouseInElement(compass));

watch(
  () => mouse.isOutside,
  (isOutside) => {
    if (isOutside) {
      uiStore.ui.isMouseInCompass = false;
    }
  }
);

// Request auto rotation from mouse click in compass
// -------------------------------------------------
function requestAngle() {
  if (!settingsStore.isLockedByElse) {
    const needle = document.getElementById('cmp-req-needle');
    needle.style.filter = 'brightness(120%)';
    setTimeout(() => {
      needle.style.filter = 'unset';
    }, 100);
    if (uiStore.ui.isMouseInCompass) {
      umbrellaStore.sendTarget(uiStore.ui.requestAngle.toFixed(1));
    }
  }
}

// Calc positions of favorites dots
// --------------------------------

// Adjust position of favorite dots depending on
// wether the degree and cardinal labels are shown
const favoritesRingRadius = computed(() => {
  let radius = 245;
  if (!uiStore.ui.hasCardinalLabels) {
    radius = 301;
  }
  if (!uiStore.ui.hasDegreeLabels) {
    radius = 443;
  }
  return radius;
});

// Dot X-coordinate
function favoriteXCoordinate(angle) {
  return 500 + Math.sin((angle / 180) * Math.PI) * favoritesRingRadius.value;
}

// Dot Y-coordinate
function favoriteYCoordinate(angle) {
  return 500 - Math.cos((angle / 180) * Math.PI) * favoritesRingRadius.value;
}

// Color of favorite rings depends on position
// -> 378 is the radius of the main compass ring
const favoritesRingColor = computed(() => {
  if (favoritesRingRadius.value < 378) {
    return 'var(--compass-cross-color)';
  } else {
    return 'var(--compass-background-color)';
  }
});
</script>

<style lang="scss" scoped>
#compass-svg {
  position: relative;
  z-index: 1;
}

.mouse-inside {
  cursor: pointer;
}

.locked {
  cursor: not-allowed;
}

/* Needles & OL-Indicator */
/* ====================== */
#cmp-needle,
#cmp-req-needle,
#cmp-target-needle {
  transform-origin: 500px 500px;
  transform: rotate(0deg);
}

#cmp-needle {
  transition: transform 0.1s linear;
}

#cmp-target-needle {
  opacity: 0.3;
}

#cmp-req-needle {
  transition: opacity 0.1s;

  @include touch {
    opacity: 0;
  }
}

.hide-opacity {
  opacity: 0;
}

#cmp-ol-indicator {
  transition: opacity 0.5s;
}

/* Labels in Compass */
/* ================= */
#cmp-center {
  text {
    text-anchor: middle;
    font-size: 34px;
    transform: opacity 0.1s;
  }

  text:nth-child(3) {
    font-size: 49px;
  }
}

#cmp-cardinals {
  font-size: 42px;
}

#cmp-degrees {
  text-anchor: middle;
  font-size: 36px;
  cursor: pointer;

  text:hover,
  text:focus-visible {
    font-weight: bold;
  }

  text:active {
    font-weight: normal;
  }
}

.cmp-favorite-dot {
  text-anchor: middle;
  font-size: 28px;
  cursor: pointer;

  &:hover {
    font-weight: bold;
  }
  &:active {
    font-weight: normal;
  }
}

.cmp-lbl-disabled {
  cursor: not-allowed;
  &:hover {
    font-weight: unset !important;
  }
}

/* Corner Labels */
/* ============= */
.corner-lbl {
  position: absolute;
  z-index: -1;
  padding: 0.25em;
  border-radius: 0.5em;
  border: 0.2em solid var(--text-color);

  @include large {
    font-size: 1rem;
  }
}

.rotation-lbl {
  top: 0;
  left: 0;
  width: 4em;

  .rotate-cw {
    animation: 5s linear infinite rotate-cw;
  }

  .rotate-ccw {
    animation: 5s linear reverse infinite rotate-cw;
  }

  @keyframes rotate-cw {
    0% {
      transform: rotate(0deg);
    }
    100% {
      transform: rotate(360deg);
    }
  }
}

#overlap-lbl {
  top: 0;
  right: 0;
  width: 2.3em;
  color: var(--alert-color);
  border-color: var(--alert-color);
}

#lock-lbl {
  bottom: 0;
  right: 0;
  width: 2em;
  color: var(--alert-color);
  border-color: var(--alert-color);
  animation: 1s ease infinite alternate blink;

  @keyframes blink {
    0% {
      color: var(--text-color);
      border-color: var(--text-color);
    }
    100% {
      color: var(--alert-color);
      border-color: var(--alert-color);
    }
  }
}

/* Bottom Card */
.compass-label {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr;
  align-items: center;
  justify-content: center;
  column-gap: 1em;
  row-gap: 0.5em;
}
</style>
