import CoreML
import CoreImage
import OpenGL.GL.Macro

public class CoreMLBackend {
    var model: MLModel?
    var inputDescriptor: String?
    var outputDescriptor: String?
    var inputWidth: Int?
    var inputHeight: Int?

    public init() {}

    public func loadModel(path: String) -> Int {
        let modelURL = URL(fileURLWithPath: path)
        do {
            let compiledModelURL = try MLModel.compileModel(at: modelURL)
            self.model = try MLModel(contentsOf: compiledModelURL)
            self.inputDescriptor = self.model!.modelDescription.inputDescriptionsByName.keys.first  
            self.outputDescriptor = self.model!.modelDescription.outputDescriptionsByName.keys.first
            if self.inputDescriptor == nil || self.outputDescriptor == nil {
                print("[ERROR] CoreMLBackend: input or output descriptor not found")
                return -1
            }
            let inputDescription = self.model!.modelDescription.inputDescriptionsByName[self.inputDescriptor!]!
            //let outputDescription = self.model!.modelDescription.outputDescriptionsByName[self.outputDescriptor!]!
            self.inputWidth = inputDescription.imageConstraint?.pixelsWide
            self.inputHeight = inputDescription.imageConstraint?.pixelsHigh

            if self.inputWidth == nil || self.inputHeight == nil {
                print("[ERROR] CoreMLBackend: input width and height not found for descriptor \(self.inputDescriptor!)")
                return -1
            }

        } catch (let error) {
            print("[ERROR] CoreMLBackend: \(error)")
            return -1
        }
        return 0
    }

    public func run(input: CVPixelBufferWrapper, outputBuffer: UnsafeMutableRawPointer) -> Int {
        if let model = self.model {
            let pixelBuffer = createCVPixelBufferFromWrapper(cvPixelBufferWrapper: input) 

            let inputFeatureProvider = CoreMLBackendInput(pixelBuffer: pixelBuffer, featureName: self.inputDescriptor!)
            do {
                let outputProvider = try model.prediction(from: inputFeatureProvider)
                let outputFeature = outputProvider.featureValue(for: outputDescriptor!)!  
                let outputMultiArray = outputFeature.multiArrayValue!
                //TODO: without memcpy
                outputMultiArray.dataPointer.withMemoryRebound(to: Float32.self, capacity: outputMultiArray.count) { ptr in
                    memcpy(outputBuffer, ptr, outputMultiArray.count * MemoryLayout<Float32>.size)
                }
                return 0
            } catch (let error) {
                print("[ERROR] CoreMLBackend: \(error)")
            }
        } else {
            print("[ERROR] CoreMLBackend: model is nil")
        }
        return -1
    }

    public func getInputWidth() -> Int {
        if self.inputWidth == nil {
            print("[ERROR] CoreMLBackend: input width not found")
            return 0
        }
        return self.inputWidth!
    }

    public func getInputHeight() -> Int {
        if self.inputHeight == nil {
            print("[ERROR] CoreMLBackend: input height not found")
            return 0
        }
        return self.inputHeight!
    }
}

public func createCoreMLBackend() -> CoreMLBackend {
    return CoreMLBackend()
}

public struct CVPixelBufferWrapper {
    let width: Int
    let height: Int
    let data: UnsafeMutableRawPointer

    public init(width: Int, height: Int, data: UnsafeMutableRawPointer) {
        self.width = width
        self.height = height
        self.data = data
    }
}

internal func createCVPixelBufferFromWrapper(cvPixelBufferWrapper: CVPixelBufferWrapper) -> CVPixelBuffer {
    var pixelBuffer: CVPixelBuffer!
    let status = CVPixelBufferCreateWithBytes(
        kCFAllocatorDefault,
        cvPixelBufferWrapper.width,
        cvPixelBufferWrapper.height,
        kCVPixelFormatType_32BGRA,
        cvPixelBufferWrapper.data,
        cvPixelBufferWrapper.width * 4,
        nil,
        nil,
        nil,
        &pixelBuffer
    )
    if status != kCVReturnSuccess {
        fatalError("[ERROR] Failed to create pixel buffer")
    }
    return pixelBuffer
}

internal class CoreMLBackendInput: MLFeatureProvider {
    let pixelBuffer: CVPixelBuffer
    let featureName: String

    init(pixelBuffer: CVPixelBuffer, featureName: String) {
        self.pixelBuffer = pixelBuffer
        self.featureName = featureName
    }

    var featureNames: Set<String> {
        return [self.featureName]
    }

    func featureValue(for featureName: String) -> MLFeatureValue? {
        if featureName == self.featureName {
            return MLFeatureValue(pixelBuffer: self.pixelBuffer)
        }
        return nil
    }
}

// debug helpers
internal func getDocumentsDirectory() -> URL {
    let paths = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
    return paths[0]
}
internal func saveImage(pixelBuffer: CVPixelBuffer) {
    let ciimage = CIImage(cvPixelBuffer: pixelBuffer)
    let context = CIContext()
    let url = getDocumentsDirectory().appendingPathComponent("test.png")
    try! context.writePNGRepresentation(of: ciimage, to: url, format: CIFormat.ARGB8, colorSpace: ciimage.colorSpace!)
}
