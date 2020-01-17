using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using UnityEngine.UI;
using System;
using UnityEngine.Rendering;


public class DataIndex
{
    private IntPtr dataIndex;

    [DllImport("MinVR_Externald")]
    private static extern void dataIndexToString([In, Out] IntPtr data, IntPtr dataIndex);
    [DllImport("MinVR_Externald")]
    private static extern int dataIndexGetName(IntPtr dataIndex, [In, Out] IntPtr data);
    [DllImport("MinVR_Externald")]
    private static extern int dataIndexGetIntValue(IntPtr dataIndex, string key);
    [DllImport("MinVR_Externald")]
    private static extern float dataIndexGetFloatValue(IntPtr dataIndex, string key);
    [DllImport("MinVR_Externald")]
    private static extern void dataIndexGetFloatArray(IntPtr dataIndex, string key, [In, Out] IntPtr arr, int size);

    public DataIndex(IntPtr dataIndex)
    {
        this.dataIndex = dataIndex;
    }

    public string getName()
    {
        byte[] data = new byte[50];
        GCHandle h = GCHandle.Alloc(data, GCHandleType.Pinned);
        int len = dataIndexGetName(dataIndex, h.AddrOfPinnedObject());
        h.Free();
        return System.Text.Encoding.UTF8.GetString(data).Substring(0, len);
    }

    public int getInt(string key) {
        return dataIndexGetIntValue(dataIndex, key);
    }

    public float getFloat(string key)
    {
        return dataIndexGetFloatValue(dataIndex, key);
    }

    public float[] getFloatArray(string key, int size)
    {
        float[] arr = new float[size];
        GCHandle h = GCHandle.Alloc(arr, GCHandleType.Pinned);
        dataIndexGetFloatArray(dataIndex, key, h.AddrOfPinnedObject(), size);
        h.Free();
        return arr;
    }

    public Matrix4x4 getMatrix4x4(string key)
    {
        float[] arr = getFloatArray(key, 16);
        Matrix4x4 m = new Matrix4x4();
        for (int f = 0; f < arr.Length; f++)
        {
            m[f] = arr[f];
        }
        return m;
        //return Matrix4x4.identity;
    }

    public override string ToString()
    {
        byte[] data = new byte[5000];
        GCHandle h = GCHandle.Alloc(data, GCHandleType.Pinned);
        dataIndexToString(h.AddrOfPinnedObject(), dataIndex);
        h.Free();
        var str = System.Text.Encoding.UTF8.GetString(data).Trim();
        return str;
    }
}

public class MinVR : MonoBehaviour
{
    private delegate void MinVRCallback(IntPtr app, IntPtr dataIndex);
    private MinVRCallback eventCallback;
    private MinVRCallback contextCallback;
    private MinVRCallback sceneCallback;

    //Lets make our calls from the Plugin
    [DllImport("MinVR_Externald")] 
    private static extern int testDLL();
    [DllImport("MinVR_Externald")]
    private static extern IntPtr createApp(string config, MinVRCallback eventCallback, MinVRCallback contextCallback, MinVRCallback sceneCallback);
    [DllImport("MinVR_Externald")]
    private static extern IntPtr frame(IntPtr app);
    [DllImport("MinVR_Externald")]
    private static extern void destroyApp(IntPtr app);
    [DllImport("VLOpenGLConnector")]
    private static extern IntPtr createDisplayManager();
    [DllImport("VLOpenGLConnector")]
    private static extern void createDisplay(IntPtr displayManager, string name, int width, int height, int xPos, int yPos, int device);
    [DllImport("VLOpenGLConnector")]
    private static extern void startDisplay(IntPtr displayManager);
    [DllImport("VLOpenGLConnector")]
    private static extern void destroyDisplayManager(IntPtr displayManager);
    [DllImport("VLOpenGLConnector")]
    private static extern IntPtr GetCreateTexturesFunc();
    [DllImport("VLOpenGLConnector")]
    private static extern int getTexture(IntPtr displayManager, int index);
    [DllImport("VLOpenGLConnector")]
    private static extern int syncFrame(IntPtr displayManager);

    public string config;
    private IntPtr app;
    List<GameObject> views = new List<GameObject>();
    int sceneIndex;
    List<RenderTexture> renderTextures = new List<RenderTexture>();
    private IntPtr displayManager;
    bool displaysAreRunning = false;
    List<Texture2D> externalTextures = new List<Texture2D>();
    int frameNum = 0;

    void OnMinVREvent(IntPtr app, IntPtr dataIndex)
    {
        DataIndex eventData = new DataIndex(dataIndex);
        string eventName = eventData.getName();
        /*Debug.Log(eventName);
        if (String.Equals(eventName, "FrameStart"))
        {
            Debug.Log(eventName + " " + eventData.getFloat("ElapsedSeconds"));
        }*/
        //Debug.Log(index.ToString());
    }

    void OnRenderContext(IntPtr app, IntPtr dataIndex)
    {
        //Debug.Log("context callback" + dataIndex);
    }

    void OnRenderScene(IntPtr app, IntPtr dataIndex)
    {
        DataIndex index = new DataIndex(dataIndex);
        bool initRender = index.getInt("InitRender") == 1;
        Camera cam = null;
        if (initRender)
        {
            Debug.Log(index.ToString());
            GameObject view = new GameObject(index.getName());
            view.transform.parent = transform;
            cam = view.AddComponent(typeof(Camera)) as Camera;
            views.Add(view);

            //RenderTexture rt = new RenderTexture(index.getInt("WindowWidth"), index.getInt("WindowHeight"), 24, RenderTextureFormat.ARGB32);
            RenderTexture rt = new RenderTexture(256, 256, 24, RenderTextureFormat.ARGB32);
            rt.Create();
            renderTextures.Add(rt);

            cam.targetTexture = rt;

            createDisplay(displayManager, "" + index.getInt("WindowID"), index.getInt("WindowWidth"), index.getInt("WindowHeight"), index.getInt("WindowX"), index.getInt("WindowY"), 0);
        }
        else
        {
            cam = views[sceneIndex].GetComponent<Camera>();
        }

        Matrix4x4 proj = index.getMatrix4x4("ProjectionMatrix");
        cam.projectionMatrix = proj;
        cam.worldToCameraMatrix = index.getMatrix4x4("ViewMatrix");
        Matrix4x4 rh_to_lh = Matrix4x4.identity;
        rh_to_lh[0, 0] = -1;
        cam.worldToCameraMatrix = cam.worldToCameraMatrix * rh_to_lh;
        
        sceneIndex++;
    }
  

    // Use this for initialization
    IEnumerator Start()
    {
        eventCallback = new MinVRCallback(OnMinVREvent);
        contextCallback = new MinVRCallback(OnRenderContext);
        sceneCallback = new MinVRCallback(OnRenderScene);
        app = createApp(config, eventCallback, contextCallback, sceneCallback);
        displayManager = createDisplayManager();
        //Debug.Log(testDLL());
        yield return StartCoroutine("SyncFrames");
    }

    // Update is called once per frame
    void Update()
    {
        sceneIndex = 0;
        if (displaysAreRunning && externalTextures.Count == 0) {
            for (int f = 0; f < 8; f++)
            {
                System.IntPtr pointer = new System.IntPtr(getTexture(displayManager, f));
                Texture2D tex = Texture2D.CreateExternalTexture(256, 256, TextureFormat.RGBA32, false, false, pointer);
                Debug.Log(f);
                externalTextures.Add(tex);
            }
            for (int f = 0; f < views.Count; f++)
            {
                Camera cam = views[f].GetComponent<Camera>();
                CommandBuffer commandBuffer = new CommandBuffer();
                commandBuffer.CopyTexture(renderTextures[f], externalTextures[f * 2]);
                cam.AddCommandBuffer(CameraEvent.AfterEverything, commandBuffer);
            }
        }
        frame(app);
        if (!displaysAreRunning)
        {
            GL.IssuePluginEvent(GetCreateTexturesFunc(), 1);
            startDisplay(displayManager);
            displaysAreRunning = true;
        }
        else
        {
            /*for (int f = 0; f < renderTextures.Count; f++)
            //for (int f = renderTextures.Count-1; f >= 1; f--)
            {
                Graphics.CopyTexture(renderTextures[f], externalTextures[f * 2 + ((frameNum + 1) % 2)]);
            }*/
        }
        frameNum++;
    }

    void OnGUI()
    {
        /*for (int f = 0; f < renderTextures.Count; f++)
        //for (int f = renderTextures.Count-1; f >= 1; f--)
        {
            Graphics.CopyTexture(renderTextures[f], externalTextures[f * 2 + ((frameNum + 1) % 2)]);
        }*/
    }
    
    void OnApplicationQuit()
    {
        Debug.Log("Quit");
        destroyApp(app);
        destroyDisplayManager(displayManager);
    }


    private IEnumerator SyncFrames()
    {
        int frameNum = 0;
        while (true)
        {
            // Wait until all frame rendering is done
            yield return new WaitForEndOfFrame();
            Debug.Log("Frame: " + frameNum);
            /*for (int f = 0; f < renderTextures.Count; f++)
            {
                Graphics.CopyTexture(renderTextures[f], externalTextures[f * 2]);
            }*/
            /*for (int f = 0; f < renderTextures.Count; f++)
            {
                Graphics.CopyTexture(renderTextures[f], externalTextures[f * 2 + ((frameNum) % 2)]);
            }*/
            int f = syncFrame(displayManager);
            Debug.Log("Ext Frame: " + f);
            
            frameNum++;
        }
    }
}
