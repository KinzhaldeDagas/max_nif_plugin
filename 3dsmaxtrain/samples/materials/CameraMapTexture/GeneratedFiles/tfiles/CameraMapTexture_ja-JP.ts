<?xml version="1.0" encoding="utf-8"?><!DOCTYPE TS><TS version="2.1" language="ja_JP">
<context>
    <name>CameraMapRollup</name>
    <message>
        <location filename="../../CameraMap.ui" line="20"/>
        <source>ZBuffer</source>
        <translation>Z バッファ</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="27"/>
        <source>Camera:</source>
        <translation>カメラ:</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="37"/>
        <source>Alpha</source>
        <translation>アルファ</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="44"/>
        <source>Map Channel:</source>
        <translation>マップ チャネル:</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="54"/>
        <source>Mask:</source>
        <translation>マスク:</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="61"/>
        <source>Remove Back Face Pixels</source>
        <translation>背面ピクセルを除去</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="68"/>
        <source>ZBuffer Mask:</source>
        <translation>Z バッファ マスク:</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="75"/>
        <source> °</source>
        <translation> °</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="88"/>
        <source>Pick Camera</source>
        <translation>カメラを選択</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="95"/>
        <source>Texture</source>
        <translation>テクスチャ</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="102"/>
        <source>ZFudge:</source>
        <translation>Z ファッジ:</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="112"/>
        <source>Mask Uses the Camera Projection</source>
        <translation>カメラ投影を使用したマスク</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="119"/>
        <source>Texture:</source>
        <translation>テクスチャ:</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="129"/>
        <source>Aspect Ratio</source>
        <translation>アスペクト比</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="135"/>
        <source>&lt;b&gt;Attached Bitmap:&lt;/b&gt;&lt;br/&gt;
In this mode, the aspect ratio of any attached &lt;b&gt;Bitmap&lt;/b&gt; is automatically used. However, if any other type of texture is used, from which the inherent aspect ratio can not be deduced, the &lt;b&gt;Custom&lt;/b&gt; aspect ratio is used as a fallback.</source>
        <translation>&lt;b&gt;アタッチされたビットマップ:&lt;/b&gt;&lt;br/&gt;
このモードでは、アタッチされた&lt;b&gt;ビットマップ&lt;/b&gt;のアスペクト比が自動的に使用されます。ただし、別のタイプのテクスチャが使用されていて、そこから固有のアスペクト比を推測できない場合は、&lt;b&gt;カスタム&lt;/b&gt;のアスペクト比が代替として使用されます。</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="139"/>
        <source>Attached Bitmap (when available)</source>
        <translation>アタッチされたビットマップ(使用可能な場合)</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="149"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Vertical Aspect Ratio&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;垂直アスペクト比&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="156"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Horizontal Aspect Ratio&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;水平アスペクト比&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="163"/>
        <source>/</source>
        <translation>/</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="170"/>
        <source>&lt;b&gt;Custom:&lt;/b&gt;&lt;br/&gt;
Set any custom width-to-height ratio for use as the projections aspect ratio. This ratio is also used if the &lt;b&gt;Attached Bitmap&lt;/b&gt; mode is selected, but the attached texture &lt;i&gt;isn't&lt;/i&gt; a pure &lt;b&gt;Bitmap&lt;/b&gt;.</source>
        <translation>&lt;b&gt;カスタム:&lt;/b&gt;&lt;br/&gt;
投影のアスペクト比として使用するカスタムの幅/高さ比を設定します。&lt;b&gt;アタッチされたビットマップ&lt;/b&gt; モードが選択されている場合もこの比率が使用されますが、アタッチされたテクスチャは純粋な&lt;i&gt;&lt;/i&gt;&lt;b&gt;ビットマップ&lt;/b&gt;ではありません。</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="174"/>
        <source>Custom:</source>
        <translation>カスタム:</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="187"/>
        <source>&lt;b&gt;Rendered Image:&lt;/b&gt;&lt;br/&gt;
Matches the aspect ratio of the image currently being rendered, which is the legacy behavior. In this mode, there is a risk of the projection changing when output resolution is changed.</source>
        <translation>&lt;b&gt;レンダリング イメージ:&lt;/b&gt;&lt;br/&gt;
現在レンダリングされているイメージのアスペクト比を照合します(従来の動作)。このモードでは、出力解像度が変更された場合、投影が変化する可能性があります。</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="191"/>
        <source>Rendered Image (Legacy Mode)</source>
        <translation>レンダリング イメージ(従来のモード)</translation>
    </message>
    <message>
        <location filename="../../CameraMap.ui" line="204"/>
        <source>Angle Threshold:</source>
        <translation>角度しきい値:</translation>
    </message>
</context>
</TS>